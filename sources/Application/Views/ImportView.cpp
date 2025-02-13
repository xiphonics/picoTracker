#include "ImportView.h"

#include "Application/AppWindow.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "ModalDialogs/MessageBox.h"
#include "pico/multicore.h"
#include <memory>
#include <nanoprintf.h>

#define LIST_WIDTH SCREEN_WIDTH - 2
// -4 to allow for title, filesize & spacers
#define LIST_PAGE_SIZE SCREEN_HEIGHT - 4

ImportView::ImportView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {}

ImportView::~ImportView() {}

void ImportView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_PLAY) {
    auto picoFS = PicoFileSystem::GetInstance();
    char name[PFILENAME_SIZE];
    unsigned fileIndex = fileIndexList_[currentIndex_];
    picoFS->getFileName(fileIndex, name, PFILENAME_SIZE);

    if (mask & EPBM_ALT) {
      Trace::Log("PICOIMPORT", "SHIFT play - import");
      import(name);
    } else {
      Trace::Log("PICOIMPORT", "plain play preview");
      preview(name);
    }
    // handle moving up and down the file list
  } else if (mask & EPBM_UP) {
    warpToNextSample(true);
  } else if (mask & EPBM_DOWN) {
    warpToNextSample(false);
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_NAV)) {
    // Go to back "left" to instrument screen
    ViewType vt = VT_INSTRUMENT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
  } else {
    // A modifier
    if (mask & EPBM_ENTER) {
      auto picoFS = PicoFileSystem::GetInstance();
      unsigned fileIndex = fileIndexList_[currentIndex_];
      char name[PFILENAME_SIZE];
      picoFS->getFileName(fileIndex, name, PFILENAME_SIZE);
      if (picoFS->getFileType(fileIndex) == PFT_DIR) {
        setCurrentFolder(picoFS, name);
        isDirty_ = true;
        topIndex_ = 0; // need to reset when entering a dir as prev dir may
                       // have been already scrolled down
      }
    }
  }
};

void ImportView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  auto picoFS = PicoFileSystem::GetInstance();

  // Draw title
  const char *title = "Import Sample";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);

  SetColor(CD_NORMAL);

  drawBattery(props);

  // Draw samples
  int x = 1;
  int y = pos._y + 2;

  // need to use fullsize buffer as sdfat doesnt truncate if filename longer
  // than buffer but instead returns empty string  in buffer :-(
  char buffer[PFILENAME_SIZE];
  for (size_t i = topIndex_;
       i < topIndex_ + LIST_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
    if (i == currentIndex_) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    }

    memset(buffer, '\0', sizeof(buffer));
    unsigned fileIndex = fileIndexList_[i];

    if (picoFS->getFileType(fileIndex) != PFT_DIR) {
      picoFS->getFileName(fileIndex, buffer, PFILENAME_SIZE);
    } else {
      buffer[0] = '/';
      picoFS->getFileName(fileIndex, buffer + 1, PFILENAME_SIZE);
    }
    // make sure truncate to list width the filename with trailing null
    buffer[LIST_WIDTH] = 0;
    DrawString(x, y, buffer, props);
    y += 1;
  };

  // draw current selected file size
  SetColor(CD_HILITE2);
  props.invert_ = true;
  y = 0;
  auto currentFileIndex = fileIndexList_[currentIndex_];
  if (picoFS->getFileType(currentFileIndex) == PFT_FILE) {
    int filesize = picoFS->getFileSize(currentFileIndex);
    npf_snprintf(buffer, sizeof(buffer), "[size: %i]", filesize);
    x = 1;  // align with rest screen title & file list
    y = 23; // bottom line
    DrawString(x, y, buffer, props);
  }

  SetColor(CD_NORMAL);
};

void ImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick){};

void ImportView::OnFocus() {
  auto picoFS = PicoFileSystem::GetInstance();

  toInstr_ = viewData_->currentInstrumentID_;

  setCurrentFolder(picoFS, SAMPLES_LIB_DIR);
};

void ImportView::warpToNextSample(bool goUp) {
  if (goUp) {
    if (currentIndex_ > 0) {
      currentIndex_--;
      // if we have scrolled off the top, page the file list up if not
      // already at  very top of the list
      if (currentIndex_ < topIndex_) {
        topIndex_ = currentIndex_;
      }
    }
  } else {
    if (currentIndex_ < fileIndexList_.size() - 1) {
      currentIndex_++;
      // if we have scrolled off the bottom, page the file list down if not
      // at end of the list
      if (currentIndex_ >= (topIndex_ + LIST_PAGE_SIZE)) {
        topIndex_++;
      }
    }
  }
  isDirty_ = true;
}

void ImportView::preview(char *name) {
  if (Player::GetInstance()->IsPlaying()) {
    Player::GetInstance()->StopStreaming();
    if (currentIndex_ != previewPlayingIndex_) {
      previewPlayingIndex_ = currentIndex_;
      Player::GetInstance()->StartStreaming(name);
    }
  } else {
    Player::GetInstance()->StartStreaming(name);
    previewPlayingIndex_ = currentIndex_;
  }
}

void ImportView::import(char *name) {
  // stop playing before trying to import
  if (Player::GetInstance()->IsPlaying()) {
    MessageBox *mb =
        new MessageBox(*this, "Can't import while previewing", MBBF_OK);
    DoModal(mb);
    return;
  }

  SamplePool *pool = SamplePool::GetInstance();

  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_start_blocking();
  char projName[MAX_PROJECT_NAME_LENGTH];
  viewData_->project_->GetProjectName(projName);
  int sampleID = pool->ImportSample(name, projName);
  multicore_lockout_end_blocking();

  if (sampleID >= 0) {
    I_Instrument *instr =
        viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
    if (instr->GetType() == IT_SAMPLE) {
      SampleInstrument *sinstr = (SampleInstrument *)instr;
      sinstr->AssignSample(sampleID);
    };
  } else {
    Trace::Error("failed to import sample");
  };
  isDirty_ = true;
};

void ImportView::setCurrentFolder(PicoFileSystem *picoFS, const char *name) {
  // Trace::Log("PICOIMPORT", "set Current Folder:%s");
  if (!picoFS->chdir(name)) {
    Trace::Error("FAILED to chdir to %s", name);
  }
  currentIndex_ = 0;
  // now update list of file indexes in this new dir
  picoFS->list(&fileIndexList_, ".wav", false);

  bool isSampleLibDir = picoFS->isParentRoot();
  if (isSampleLibDir && !fileIndexList_.empty()) {
    fileIndexList_.erase(fileIndexList_.begin());
  }
}
