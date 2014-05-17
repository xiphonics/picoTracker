#include "MessageBox.h"

static const char *buttonText[MBL_LAST] = {
	"Ok",
	"Yes",
	"Cancel",
	"No"
} ;

MessageBox::MessageBox(View &view,const char *message,int btnFlags):
	ModalView(view),
	message_(message) {

	buttonCount_=0 ;
	for (int i=0;i<MBL_LAST;i++) {
		if (btnFlags&(1<<(i))) {
			button_[buttonCount_]=i ;
			buttonCount_++ ;
		}
	}
	selected_=buttonCount_-1 ;
	NAssert(buttonCount_!=0) ;
} ;

MessageBox::~MessageBox() {
} ;

void MessageBox::DrawView() {
// message size
	int size=message_.size() ;

// compute space needed for buttons
// and set window size

	int btnSize=5 ;
	int width=buttonCount_*(btnSize+1)+1 ;
	width=(size>width)?size:width ;
	SetWindow(width,3) ;

// draw text

	int y=0 ;
	int x=(width-size)/2;
	GUITextProperties props ;
	SetColor(CD_NORMAL) ;
	DrawString(x,y,message_.c_str(),props) ;
	
	y=2 ;
	int offset=width/(buttonCount_+1) ;

	for (int i=0;i<buttonCount_;i++) {
		const char *text=buttonText[button_[i]] ;
		x=offset*(i+1)-strlen(text)/2 ;
		props.invert_=(i==selected_)?true:false ;
		DrawString(x,y,text,props) ;
	}	
} ;

void MessageBox::OnPlayerUpdate(PlayerEventType ,unsigned int currentTick) {
} ;
void MessageBox::OnFocus() {
} ;
void MessageBox::ProcessButtonMask(unsigned short mask,bool pressed) {
	if (mask&EPBM_A) {
		EndModal(button_[selected_]) ;
	}
	if (mask&EPBM_LEFT) {
		selected_=(selected_+1) ;
		if (selected_>=buttonCount_) {
			selected_=0 ;
		}
	} 
	if (mask&EPBM_RIGHT) {
		selected_=(selected_-1) ;
		if (selected_<0) {
			selected_=buttonCount_-1 ;
		}
	} 
	isDirty_=true ;
} ;

