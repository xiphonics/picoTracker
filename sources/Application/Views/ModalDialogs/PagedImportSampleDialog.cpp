#include "PagedImportSampleDialog.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_SIZE 15
#define LIST_WIDTH 24

static const char *buttonText[3]= {
	"Listen",
	"Import",
	"Exit"	
} ;

PagedImportSampleDialog::PagedImportSampleDialog(View &view):ModalView(view) {
	selected_=0 ;
}

PagedImportSampleDialog::~PagedImportSampleDialog() {
  // TODO: clear all char*'s in fileList_ array
}

void PagedImportSampleDialog::DrawView() {
	SetWindow(LIST_WIDTH,LIST_SIZE+3) ;
	GUITextProperties props ;

// Draw title
#ifdef SHOW_MEM_USAGE
	char title[40] ;

	SetColor(CD_NORMAL) ;

	sprintf(title,"MEM [%d]", System::GetInstance()->GetMemoryUsage()) ;
	GUIPoint pos = GUIPoint(0, 0);
	w_.DrawString(title, pos, props);

#endif
// Draw samples
	int x= 1;
	int y= 1;
	if (currentSample_ < topIndex_) {
		topIndex_ = currentSample_;
	}
	if (currentSample_>= topIndex_+LIST_SIZE) {
		topIndex_=currentSample_;
	}
	int count = 0;
	char buffer[64];
	for(FileListItem current : fileList_) {
		if ((count >= topIndex_) && (count < topIndex_ + LIST_SIZE)) {
			if (count==currentSample_) {
				SetColor(CD_HILITE2);
				props.invert_=true;
			} else {
				SetColor(CD_NORMAL);
				props.invert_ = false;
			}
			if (!current.IsDirectory) {
				strcpy(buffer, current.name);
			} else {
				buffer[0]='[' ;
				strcpy(buffer + 1, current.name);
				strcat(buffer,"]");
			}
			// now "truncate" the buffer to LIST_WIDTH
			buffer[LIST_WIDTH - 1] = 0;
			DrawString(x, y, buffer, props);
			y += 1;
		}
		count++;
	} ;

	y = LIST_SIZE+2;
	int offset = 7;

	SetColor(CD_NORMAL) ;

	for (int i = 0; i < 3; i++) {
		const char *text=buttonText[i];
		x=(offset*(i+1)-strlen(text)/2) - 2;
		props.invert_ = (i==selected_) ? true : false;
		DrawString(x, y, text, props) ;
	}	
} ;

void PagedImportSampleDialog::warpToNextSample(int direction) {

	currentSample_+=direction ;
	int size= fileList_.size() ; 
	if (currentSample_<0) currentSample_+=size ;
	if (currentSample_>=size) currentSample_-=size ;
	isDirty_=true ;
}

void PagedImportSampleDialog::OnPlayerUpdate(PlayerEventType ,unsigned int currentTick) {
} ;

void PagedImportSampleDialog::OnFocus() {
	Path current(currentPath_) ;
	setCurrentFolder(&current) ;
	toInstr_=viewData_->currentInstrument_ ;
} ;

void PagedImportSampleDialog::preview(Path &element) {
	Player::GetInstance()->StartStreaming(element) ;
}

void PagedImportSampleDialog::import(Path &element) {

	SamplePool *pool=SamplePool::GetInstance() ;

#ifdef PICO_BUILD
  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_start_blocking();
#endif
  int sampleID = pool->ImportSample(element);
#ifdef PICO_BUILD
  multicore_lockout_end_blocking();
#endif

  if (sampleID >= 0) {
    I_Instrument *instr =
      viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
    if (instr->GetType() == IT_SAMPLE) {
      SampleInstrument *sinstr = (SampleInstrument *)instr;
      sinstr->AssignSample(sampleID);
      toInstr_ = viewData_->project_->GetInstrumentBank()->GetNext();
    };
	} else {
		Trace::Error("failed to import sample") ;
	};
	isDirty_=true ;
} ;

void PagedImportSampleDialog::ProcessButtonMask(unsigned short mask, bool pressed) {

	if (!pressed) return ;

	if (mask&EPBM_B) {  
		if (mask&EPBM_UP) warpToNextSample(-LIST_SIZE) ;
		if (mask&EPBM_DOWN) warpToNextSample(LIST_SIZE) ;
	} else {
		// A modifier
		if (mask&EPBM_A) {
			FileListItem selected = fileList_[selected_];
			
			if (selected.IsDirectory) {
				if (selected.name == std::string("..")) {
					if (currentPath_.GetPath() == std::string(sampleLib_)) {
						// if already at root of samplelib do nothing
					} else {
						Path parent = currentPath_.GetParent();
						setCurrentFolder(&parent);
					}
				} else {
					Path childDir = currentPath_.Descend(std::string(selected.name));
					setCurrentFolder(&childDir);
				}
				isDirty_=true ;
				return ;
			}
			auto fullPathStr = std::string(currentPath_.GetPath());
			fullPathStr += "/";
			fullPathStr += selected.name;
			auto fullPath = Path { fullPathStr };
			switch(selected_) {
				case 0: // preview
					preview(fullPath);
					break ;
				case 1: // import
					import(fullPath) ;
					break ;
				case 2: // Exit ;
					EndModal(0) ;
					break ;
			}
		} else {
			// R Modifier
			if (mask&EPBM_R) {
			} else {
				// No modifier
				if (mask==EPBM_UP) warpToNextSample(-1) ;
				if (mask==EPBM_DOWN) warpToNextSample(1) ;
				if (mask==EPBM_LEFT) {
					selected_-=1 ;
					if (selected_<0) selected_+=3 ;
					isDirty_=true ;
				}
				if (mask==EPBM_RIGHT) {
					selected_=(selected_+1)%3 ;
					isDirty_=true ;
				}
			}
		} 
	}
}

void PagedImportSampleDialog::setCurrentFolder(Path *path) {
	Path formerPath(currentPath_) ;
	topIndex_=0 ;
	currentSample_=0 ;
	currentPath_=Path(*path) ;
	auto result = fileList_.empty();
	if (!result) {
		Trace::Log("PagedImport", "couldn't clear fileList buffer");
	}
	if (path) {
		auto dir = FileSystem::GetInstance()->OpenPaged(path->GetPath().c_str());
		//TODO: show "Loading..." mesg in UI
		dir->GetContent("*.wav");
		//TODO: hide "Loading..." mesg in UI
	}
}
