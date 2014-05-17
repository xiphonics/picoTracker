#include "AppWindow.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "System/Console/Trace.h"
#include "Player/Player.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "Views/UIController.h"
#include "Application/Persistency/PersistencyService.h" 
#include "Application/Instruments/SamplePool.h"
#include "Application/Player/TablePlayback.h"
#include "Services/Midi/MidiService.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Commands/EventDispatcher.h"
#include "Application/Commands/ApplicationCommandDispatcher.h"
#include "Application/Views/ModalDialogs/SelectProjectDialog.h"
#include "Application/Utils/char.h"
#include "Application/Mixer/MixerService.h"
#include <string.h>

AppWindow *instance=0 ;

GUIColor AppWindow::backgroundColor_(0xF1,0xF1,0x96) ;
GUIColor AppWindow::normalColor_(0x77,0x6B,0x56) ;
GUIColor AppWindow::highlight2Color_(0x8E,0xA0,0x4A) ;
GUIColor AppWindow::highlightColor_(0xA8,0x16,0x16) ;
GUIColor AppWindow::consoleColor_(0xFF,0x00,0xFF) ;
GUIColor AppWindow::cursorColor_(0x77,0x6B,0x56) ;

int AppWindow::charWidth_=8;
int AppWindow::charHeight_=8 ;

//#define _FORCE_SDL_EVENT_

static void ProjectSelectCallback(View &v,ModalView &dialog) {

	SelectProjectDialog &spd=(SelectProjectDialog &)dialog ;
	if (dialog.GetReturnCode()>0) {
		Path selected=spd.GetSelection() ;
		instance->LoadProject(selected.GetPath().c_str()) ;
	} else {
		System::GetInstance()->PostQuitMessage() ;
	}
} ;

void AppWindow::defineColor(const char *colorName,GUIColor &color) {

       Config *config=Config::GetInstance() ;
       const char *value=config->GetValue(colorName) ;
       if (value) {
          unsigned char r ;
          char2hex(value,&r) ;
          unsigned char g ;
          char2hex(value+2,&g) ;
          unsigned char b ;
          char2hex(value+4,&b) ;
          color=GUIColor(r,g,b) ;
       }
}


AppWindow::AppWindow(I_GUIWindowImp &imp):GUIWindow(imp)  {

	instance=this ;

	// Init all members

	_statusLine[0]=0 ;

	_currentView=0 ;	
	_viewData=0 ;
	_songView=0 ;
	_chainView=0 ;
	_phraseView=0 ;
	_projectView=0 ;
	_instrumentView=0 ;
	_tableView=0 ;
	_nullView=0 ;
	_mixerView=0 ;
	_grooveView=0 ;
	_closeProject=0 ;
	_lastA=0 ;
	_lastB=0 ;
	_mask=0 ;
	colorIndex_=CD_NORMAL;

	EventDispatcher *ed=EventDispatcher::GetInstance() ;
	ed->SetWindow(this) ;

	Status::Install(this) ;

	// Init midi services
	MidiService::GetInstance()->Init() ;

    defineColor("BACKGROUND",backgroundColor_) ;
    defineColor("FOREGROUND",normalColor_) ;
	cursorColor_=normalColor_ ;
    defineColor("HICOLOR1",highlightColor_) ;
    defineColor("HICOLOR2",highlight2Color_) ;
    defineColor("CURSORCOLOR",cursorColor_) ;

	GUIWindow::Clear(backgroundColor_) ;
	
	_nullView=new NullView((*this),0) ;
	_currentView=_nullView ;
	_nullView->SetDirty(true) ;

	SelectProjectDialog *spd=new SelectProjectDialog(*_currentView) ;
	_currentView->DoModal(spd,ProjectSelectCallback) ;


	memset(_charScreen,' ',1200) ;
	memset(_preScreen,' ',1200) ;
	memset(_charScreenProp,0,1200) ;
	memset(_preScreenProp,0,1200) ;

	Redraw() ;
} ;

AppWindow::~AppWindow() {
	MidiService::GetInstance()->Close() ;
}


void AppWindow::DrawString(const char *string,GUIPoint &pos,GUITextProperties &props,bool force) {

// we know we don't have mode than 40 chars

	char buffer[41] ;
	int len=strlen(string) ;
	int offset=(pos._x<0)?-pos._x/8:0 ;
	len-=offset ;
	int available=40-((pos._x<0)?0:pos._x) ;
	len=MIN(len,available) ;
	memcpy(buffer,string+offset,len) ;
	buffer[len]=0 ;

	NAssert((pos._x<40)&&(pos._y<30)) ;
	int index=pos._x+40*pos._y ;
	memcpy(_charScreen+index,buffer,len) ;
	unsigned char prop=colorIndex_+(props.invert_?PROP_INVERT:0) ;
	memset(_charScreenProp+index,prop,len) ;
} ;

void AppWindow::Clear(bool all) {
	memset(_charScreen,' ',1200) ;
	memset(_charScreenProp,0,1200) ;
	if (all)  {
		memset(_preScreen,' ',1200) ;
		memset(_preScreenProp,0,1200) ;
	} ;
} ;

void AppWindow::ClearRect(GUIRect &r) {

	int x=r.Left() ;
	int y=r.Top() ;
	int w=r.Width();
	int h=r.Height() ;

	unsigned char *st=_charScreen+x+(40*y) ;
	unsigned char *pr=_charScreenProp+x+(40*y) ;
	for (int i=0;i<h;i++) {
		for (int j=0;j<w;j++) {
			*st++=' ' ;
			*pr++=0 ;
		}
		st+=(40-w) ;
		pr+=(40-w) ;
	}
} ;


//
// Redraws the screen and flush it.
//

void AppWindow::Redraw() {

	SysMutexLocker locker(drawMutex_) ;
	
	if (_currentView) {
		_currentView->Redraw() ;
		Invalidate() ;
	}
} ;

//
// Flush current screen to display
//

void AppWindow::Flush() {

	SysMutexLocker locker(drawMutex_) ;

	Lock() ;
	long flushStart=System::GetInstance()->GetClock() ;

	GUITextProperties props ;
	GUIPoint pos ;

	ColorDefinition color=(ColorDefinition)-1 ;
	pos._x=0 ;
	pos._y=0 ;

	int count=0 ;

	unsigned char *current=_charScreen ;
	unsigned char *previous=_preScreen ;
	unsigned char *currentProp=_charScreenProp ;
	unsigned char *previousProp=_preScreenProp ;
	for (int y=0;y<30;y++) {
		for (int x=0;x<40;x++) {
#ifndef _LGPT_NO_SCREEN_CACHE_
			if ((*current!=*previous)||(*currentProp!=*previousProp)) {
#endif
				props.invert_=(*currentProp&PROP_INVERT)!=0 ;
				if (((*currentProp)&0x7F)!=color) {
					color=(ColorDefinition)((*currentProp)&0x7F) ;
					GUIColor gcolor=normalColor_ ;
					switch (color) {
						case CD_BACKGROUND:
							gcolor=backgroundColor_ ;
							break ;
						case CD_NORMAL:
							break ;
						case CD_HILITE1:
							gcolor=highlightColor_ ;
							break ;
						case CD_HILITE2:
							gcolor=highlight2Color_ ;
							break ;
						case CD_CONSOLE:
							gcolor=consoleColor_ ;
							break ;
						case CD_CURSOR:
							gcolor=cursorColor_ ;
							break ;
	
						default:
							NAssert(0) ;
							break ;
					}
					GUIWindow::SetColor(gcolor) ;
				}
				GUIWindow::DrawChar(*current,pos,props) ;
				count++ ;
#ifndef _LGPT_NO_SCREEN_CACHE_
			}
#endif
			current++ ;
			previous++ ;
			currentProp++ ;
			previousProp++ ;
			pos._x+=AppWindow::charWidth_ ;
		}		
		pos._y+=AppWindow::charHeight_ ;
		pos._x=0 ;
	}
	long flushEnd=System::GetInstance()->GetClock() ;
  GUIWindow::Flush() ;
	Unlock() ;
	memcpy(_preScreen,_charScreen,1200) ;
	memcpy(_preScreenProp,_charScreenProp,1200) ;

} ;

void AppWindow::LoadProject(const Path &p)  {

	_root=p ;

	_closeProject=false ;

	PersistencyService *persist=PersistencyService::GetInstance() ;

	TablePlayback::Reset() ;

	Path::SetAlias("project",_root.GetPath().c_str()) ;
	Path::SetAlias("samples","project:samples") ;
	
	// Load the sample pool
	
	SamplePool *pool=SamplePool::GetInstance() ;
	
	pool->Load() ;

	Project *project=new Project() ;

	bool succeeded=persist->Load() ;
	if (!succeeded) {
		project->GetInstrumentBank()->AssignDefaults() ;
	} ;

    // Project

	WatchedVariable::Disable() ;
	
	project->GetInstrumentBank()->Init() ;

	WatchedVariable::Enable() ;

	ApplicationCommandDispatcher::GetInstance()->Init(project) ;

	// Create view data
	
	_viewData=new ViewData(project) ;

	// Create & observe the player
	Player *player=Player::GetInstance() ;
	bool playerOK=player->Init(project,_viewData) ;
	player->AddObserver(*this) ;

	// Create the controller
	UIController *controller=UIController::GetInstance() ;
	controller->Init(project,_viewData) ;

	// Create & observe all views
	_songView=new SongView((*this),_viewData,_root.GetName().c_str()) ;
	_songView->AddObserver((*this)) ;

  _chainView=new ChainView((*this),_viewData) ;
	_chainView->AddObserver((*this)) ;

  _phraseView=new PhraseView((*this),_viewData) ;
	_phraseView->AddObserver((*this)) ;

  _projectView=new ProjectView((*this),_viewData) ;
	_projectView->AddObserver((*this)) ;

  _instrumentView=new InstrumentView((*this),_viewData) ;
	_instrumentView->AddObserver((*this)) ;

  _tableView=new TableView((*this),_viewData) ;
	_tableView->AddObserver((*this)) ;

	_grooveView=new GrooveView((*this),_viewData) ;
	_grooveView->AddObserver(*this) ;

	_mixerView=new MixerView((*this),_viewData) ;
	_mixerView->AddObserver(*this) ;

	_currentView=_songView ;
	_currentView->OnFocus() ;

	if (!playerOK) {
		MessageBox *mb=new MessageBox(*_songView,"Failed to initialize audio",MBBF_OK) ;
		_songView->DoModal(mb) ;
	}
	
	Redraw() ;
}

void AppWindow::CloseProject() {

	_closeProject=false ;
	Player *player=Player::GetInstance() ;
	player->Stop() ;
	player->RemoveObserver(*this) ;
	
	player->Reset() ;

	SamplePool *pool=SamplePool::GetInstance() ;
	pool->Reset() ;

	TableHolder::GetInstance()->Reset() ;
	TablePlayback::Reset() ;

	ApplicationCommandDispatcher::GetInstance()->Close() ;

	SAFE_DELETE(_songView) ;
	SAFE_DELETE(_chainView) ;
	SAFE_DELETE(_phraseView) ;
	SAFE_DELETE(_projectView) ;
	SAFE_DELETE(_instrumentView) ;
	SAFE_DELETE(_tableView);

	UIController *controller=UIController::GetInstance() ;
	controller->Reset() ;

	SAFE_DELETE(_viewData) ;

	_currentView=_nullView ;
	_nullView->SetDirty(true) ;

	SelectProjectDialog *spd=new SelectProjectDialog(*_currentView) ;
	_currentView->DoModal(spd,ProjectSelectCallback) ;
	

} ;

AppWindow *AppWindow::Create(GUICreateWindowParams &params) {
	I_GUIWindowImp &imp=I_GUIWindowFactory::GetInstance()->CreateWindowImp(params) ;
	AppWindow *w=new AppWindow(imp) ;
	return w ;
} ;

void AppWindow::SetDirty() {
	_isDirty=true ;
} ;

bool AppWindow::onEvent(GUIEvent &event) {

	// We need to tell the app to quit once we're out of the
	// mixer lock, otherwise the windows driver will never return

	_shouldQuit=false ;

	_isDirty=false ;
		
	unsigned short v=1<<event.GetValue();

	MixerService *sm=MixerService::GetInstance() ;
	sm->Lock() ;

	switch(event.GetType())  {
 
		case ET_PADBUTTONDOWN:

 			_mask|=v ;
    	if (_currentView) _currentView->ProcessButton(_mask,true) ;
			break ;

		case ET_PADBUTTONUP:

			_mask&=(0xFFFF-v) ;
    		if (_currentView) _currentView->ProcessButton(_mask,false) ;
			break ;

		case ET_SYSQUIT:
			_shouldQuit=true ;
			break ;

		/*		case ET_KEYDOWN:
			if (event.GetValue()==EKT_ESCAPE&&!Player::GetInstance()->IsRunning()) {
				if (_currentView!=_listView) {
					CloseProject() ;
					_isDirty=true ;
				} else {
					System::GetInstance()->PostQuitMessage() ;
				};
			} ;*/

		default:
			break ;
	}
	sm->Unlock() ;

	if (_shouldQuit) {
		onQuitApp() ;
	} 
	if(_closeProject) {
		CloseProject() ;
		_isDirty=true ;
	}
#ifdef _SHOW_GP2X_
	Redraw() ;
#else
	if (_isDirty) Redraw() ;
#endif	
	return false ;
} ;

void AppWindow::onUpdate() {
//	Redraw() ;
	Flush() ;
} ;

void AppWindow::LayoutChildren() {
} ;

void AppWindow::Update(Observable &o,I_ObservableData *d) {

	ViewEvent *ve=(ViewEvent *)d ;

	switch(ve->GetType()) {

	  case VET_SWITCH_VIEW:
	  {
		ViewType *vt=(ViewType*)ve->GetData() ;
		if (_currentView) {
			_currentView->LooseFocus() ;
		}
		switch (*vt) {
			case VT_SONG:
			_currentView=_songView ;
			break ;
			case VT_CHAIN:
			_currentView=_chainView ;
			break ;
			case VT_PHRASE:
			_currentView=_phraseView ;
			break ;
			case VT_PROJECT:
			_currentView=_projectView ;
			break ;
			case VT_INSTRUMENT:
			_currentView=_instrumentView ;
			break ;
			case VT_TABLE:
			_currentView=_tableView ;
			break ;
			case VT_TABLE2:
			_currentView=_tableView ;
			break ;
			case VT_GROOVE:
			_currentView=_grooveView ;
			break ;
/*			case VT_MIXER:
			_currentView=_mixerView ;
*/			break ;
			
		}
		_currentView->SetFocus(*vt) ;
		_isDirty=true ;
		GUIWindow::Clear(backgroundColor_,true) ;
		Clear(true);
		Redraw() ;
		break ;
	  }

	  case VET_PLAYER_POSITION_UPDATE:
	  {
		PlayerEvent *pt=(PlayerEvent*)ve ;

		if (_currentView) {
			SysMutexLocker locker(drawMutex_) ;
			_currentView->OnPlayerUpdate(pt->GetType(),pt->GetTickCount()) ;
  		    Invalidate() ;
		}

		break ;
	  }

/*	  case VET_LIST_SELECT:
	  {
		char *name=(char*)ve->GetData() ;
		LoadProject(name) ;
		break ;
	  } */
	  case VET_QUIT_PROJECT:
	  {
   // defer event to after we got out of the view
		_closeProject=true ;
		break ;
	  }
	  case VET_QUIT_APP:
		_shouldQuit=true ;
		break ;
   	}

}

void AppWindow::onQuitApp() {
	Player *player=Player::GetInstance() ;
	player->Stop() ;
	player->RemoveObserver(*this) ;
	
	player->Reset() ;
	System::GetInstance()->PostQuitMessage() ;
}
void AppWindow::Print(char *line) {

//	GUIWindow::Clear(View::backgroundColor_,true) ;
	Clear() ;
	strcpy(_statusLine,line) ;
// unwrapped for gcc
	int position=40 ;
	position-=strlen(_statusLine) ;
	position/=2 ;
	GUIPoint pos(position,12) ;
//
	GUITextProperties props;
	SetColor(CD_NORMAL) ;
	DrawString(_statusLine,pos,props) ;
	char buildString[80] ;
	sprintf(buildString,"Piggy build %s%s_%s",PROJECT_NUMBER,PROJECT_RELEASE,BUILD_COUNT) ;
	pos._y=28 ;
	pos._x=(40-strlen(buildString))/2 ;
	DrawString(buildString,pos,props) ;
	Flush() ;
} ;

void AppWindow::SetColor(ColorDefinition cd) {
	colorIndex_=cd ;
} ;

