#include "ImportView.h"

#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_WIDTH 24
#define LIST_PAGE_SIZE 18

ImportView::ImportView(GUIWindow &w, ViewData *viewData) : View(w, viewData) {}

ImportView::~ImportView() {}

void ImportView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_START) {
    auto picoFS = PicoFileSystem::GetInstance();
    char name[PFILENAME_SIZE];
    unsigned fileIndex = fileIndexList_[currentIndex_];
    picoFS->getFileName(fileIndex, name, PFILENAME_SIZE);

    if (mask & EPBM_L) {
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
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_R)) {
    // Go to back "left" to instrument screen
    ViewType vt = VT_INSTRUMENT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
  } else {
    // A modifier
    if (mask & EPBM_A) {
      auto picoFS = PicoFileSystem::GetInstance();
      unsigned fileIndex = fileIndexList_[currentIndex_];
      char name[PFILENAME_SIZE];
      picoFS->getFileName(fileIndex, name, PFILENAME_SIZE);
      if (picoFS->getFileType(fileIndex) == PFT_DIR) {
        setCurrentFolder(picoFS, name);
        isDirty_ = true;
      }
    }
  }
};

void ImportView::DrawView() {
  Clear();
  // Trace::Log("PICOSAMPLEIMPORT", "DrawView current:%d topIdx:%d",
  // currentIndex_,
  //            topIndex_);

  GUITextProperties props;

  auto picoFS = PicoFileSystem::GetInstance();

  // Draw samples
  int x = 0;
  int y = 0;

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

  SetColor(CD_NORMAL);

  // GUITextProperties props;
  // SetColor(CD_HILITE2);

  // char buildString[80];
  // sprintf(buildString, "picoTracker build %s%s_%s", PROJECT_NUMBER,
  //         PROJECT_RELEASE, BUILD_COUNT);
  // GUIPoint pos;
  // pos._y = 22;
  // pos._x = (32 - strlen(buildString)) / 2;
  // DrawString(pos._x, pos._y, buildString, props);
};

void ImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {

};

void ImportView::OnFocus() {
  auto picoFS = PicoFileSystem::GetInstance();

  toInstr_ = viewData_->currentInstrument_;

  setCurrentFolder(picoFS, SAMPLE_LIB);
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
  SamplePool *pool = SamplePool::GetInstance();

  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_start_blocking();
  int sampleID = pool->ImportSample(name);
  multicore_lockout_end_blocking();

  if (sampleID >= 0) {
    I_Instrument *instr =
        viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
    if (instr->GetType() == IT_SAMPLE) {
      SampleInstrument *sinstr = (SampleInstrument *)instr;
      sinstr->AssignSample(sampleID);
      toInstr_ = viewData_->project_->GetInstrumentBank()->GetNext();
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

  bool isSampleLIbDir = picoFS->isParentRoot();
  if (isSampleLIbDir && !fileIndexList_.empty()) {
    fileIndexList_.erase(fileIndexList_.begin());
  }
}
