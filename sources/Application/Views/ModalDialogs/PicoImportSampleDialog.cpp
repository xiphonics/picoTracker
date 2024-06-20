#include "PicoImportSampleDialog.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_WIDTH 24
#define SAMPLE_LIB "/samplelib"

PicoImportSampleDialog::PicoImportSampleDialog(View &view) : ModalView(view) {
  Trace::Log("PICOSAMPLEIMPORT", "samplelib is:%s", SAMPLE_LIB);
  fileIndexList_.fill(0);
}

PicoImportSampleDialog::~PicoImportSampleDialog() {
  Trace::Log("PICOSAMPLEIMPORT", "Destruct ===");
}

void PicoImportSampleDialog::DrawView() {
  // Trace::Log("PICOSAMPLEIMPORT", "DrawView current:%d topIdx:%d",
  // currentIndex_,
  //            topIndex_);

  SetWindow(LIST_WIDTH, PAGED_PAGE_SIZE);
  GUITextProperties props;

  auto picoFS = PicoFileSystem::GetInstance();

  // Draw samples
  int x = 0;
  int y = 0;

  // need to use fullsize buffer as sdfat doesnt truncate if filename longer
  // than buffer but instead returns empty string  in buffer :-(
  char buffer[PFILENAME_SIZE];
  for (size_t i = topIndex_;
       i < topIndex_ + PAGED_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
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
};

void PicoImportSampleDialog::warpToNextSample(bool goUp) {

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
      if (currentIndex_ >= (topIndex_ + PAGED_PAGE_SIZE)) {
        topIndex_++;
      }
    }
  }
  isDirty_ = true;
}

void PicoImportSampleDialog::OnPlayerUpdate(PlayerEventType,
                                            unsigned int currentTick) {};

void PicoImportSampleDialog::OnFocus() {
  auto picoFS = PicoFileSystem::GetInstance();

  toInstr_ = viewData_->currentInstrument_;

  setCurrentFolder(picoFS, SAMPLE_LIB);
};

void PicoImportSampleDialog::preview(Path &element) {
  // if (Player::GetInstance()->IsPlaying()) {
  //   Player::GetInstance()->StopStreaming();
  //   if (currentSample_ != previewPlayingIndex_) {
  //     previewPlayingIndex_ = currentSample_;
  //     Player::GetInstance()->StartStreaming(element);
  //   }
  // } else {
  //   Player::GetInstance()->StartStreaming(element);
  //   previewPlayingIndex_ = currentSample_;
  // }
}

void PicoImportSampleDialog::import(Path &element) {

  //   SamplePool *pool = SamplePool::GetInstance();

  // #ifdef PICOBUILD
  //   // Pause core1 in order to be able to write to flash and ensure core1 is
  //   // not reading from it, it also disables IRQs on it
  //   //
  //   https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  //   multicore_lockout_start_blocking();
  // #endif
  //   int sampleID = pool->ImportSample(element);
  // #ifdef PICOBUILD
  //   multicore_lockout_end_blocking();
  // #endif

  //   if (sampleID >= 0) {
  //     I_Instrument *instr =
  //         viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
  //     if (instr->GetType() == IT_SAMPLE) {
  //       SampleInstrument *sinstr = (SampleInstrument *)instr;
  //       sinstr->AssignSample(sampleID);
  //       toInstr_ = viewData_->project_->GetInstrumentBank()->GetNext();
  //     };
  //   } else {
  //     Trace::Error("failed to import sample");
  //   };
  //   isDirty_ = true;
};

void PicoImportSampleDialog::ProcessButtonMask(unsigned short mask,
                                               bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_START) {
    if (mask & EPBM_L) {
      Trace::Log("PICOIMPORT", "SHIFT play - import");
      // import(fullPath);
    } else {
      Trace::Log("PICOIMPORT", "plain play preview");
      // preview(fullPath);
    }
    // handle moving up and down the file list
  } else if (mask & EPBM_UP) {
    warpToNextSample(true);
  } else if (mask & EPBM_DOWN) {
    warpToNextSample(false);
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_R)) {
    EndModal(0);
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
}

void PicoImportSampleDialog::setCurrentFolder(PicoFileSystem *picoFS,
                                              const char *name) {
  // Trace::Log("PICOIMPORT", "set Current Folder:%s");
  if (!picoFS->chdir(name)) {
    Trace::Error("FAILED to chdir to %s", name);
  }
  currentIndex_ = 0;
  // now update list of file indexes in this new dir
  picoFS->list(&fileIndexList_, ".wav");

  bool isSampleLIbDir = picoFS->isParentRoot();
  if (isSampleLIbDir && !fileIndexList_.empty()) {
    fileIndexList_.erase(fileIndexList_.begin());
  }
}
