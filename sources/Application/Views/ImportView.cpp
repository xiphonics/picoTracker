#include "ImportView.h"

#include "Application/AppWindow.h"
#include "Application/Audio/AudioFileStreamer.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/to_string.h"
#include "ModalDialogs/MessageBox.h"
#include "pico/multicore.h"
#include <memory>
#include <nanoprintf.h>

#define LIST_WIDTH SCREEN_WIDTH - 2
// -4 to allow for title, filesize & spacers
#define LIST_PAGE_SIZE SCREEN_HEIGHT - 4

// is single cycle macro, checks for FILE size of LGPT and AKWF file formats
#define IS_SINGLE_CYCLE(x) (x == 1344 || x == 300)

ImportView::ImportView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {}

ImportView::~ImportView() {}

void ImportView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_PLAY) {
    auto fs = FileSystem::GetInstance();
    char name[PFILENAME_SIZE];
    unsigned fileIndex = fileIndexList_[currentIndex_];
    fs->getFileName(fileIndex, name, PFILENAME_SIZE);

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
      auto fs = FileSystem::GetInstance();
      unsigned fileIndex = fileIndexList_[currentIndex_];
      char name[PFILENAME_SIZE];
      fs->getFileName(fileIndex, name, PFILENAME_SIZE);
      if (fs->getFileType(fileIndex) == PFT_DIR) {
        setCurrentFolder(fs, name);
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

  auto fs = FileSystem::GetInstance();

  // Draw title
  const char *title = "Import Sample";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);

  SetColor(CD_NORMAL);

  drawBattery(props);

  // Draw samples
  int x = 1;
  int y = pos._y + 2;

  // Loop through visible files in the list
  for (size_t i = topIndex_;
       i < topIndex_ + LIST_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
    if (i == currentIndex_) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_NORMAL);
      props.invert_ = false;
    }

    unsigned fileIndex = fileIndexList_[i];
    etl::string<PFILENAME_SIZE> displayName;

    if (fs->getFileType(fileIndex) != PFT_DIR) {
      // Handle regular files
      char tempBuffer[PFILENAME_SIZE];
      fs->getFileName(fileIndex, tempBuffer, PFILENAME_SIZE);

      // Check if it's a single cycle waveform
      int filesize = fs->getFileSize(fileIndex);
      bool isSingleCycle = IS_SINGLE_CYCLE(filesize);

      // Format the display name with appropriate prefix
      if (isSingleCycle) {
        displayName = "~";
        displayName += tempBuffer;
      } else {
        displayName = " ";
        displayName += tempBuffer;
      }
    } else {
      // Handle directories
      char tempBuffer[PFILENAME_SIZE];
      displayName = "/";
      fs->getFileName(fileIndex, tempBuffer, PFILENAME_SIZE);
      displayName += tempBuffer;
    }

    // Truncate to fit display width
    if (displayName.size() > LIST_WIDTH) {
      displayName.resize(LIST_WIDTH);
    }

    DrawString(x, y, displayName.c_str(), props);
    y += 1;
  };

  // draw current selected file size and single cycle indicator
  SetColor(CD_HILITE2);
  props.invert_ = true;
  y = 0;
  auto currentFileIndex = fileIndexList_[currentIndex_];
  if (fs->getFileType(currentFileIndex) == PFT_FILE) {
    int filesize = fs->getFileSize(currentFileIndex);
    // check for LGPT or AKWF standard file sizes
    bool isSingleCycle = IS_SINGLE_CYCLE(filesize);

    // Create a temporary buffer for formatting
    char tempBuffer[PFILENAME_SIZE];

    if (isSingleCycle) {
      npf_snprintf(tempBuffer, sizeof(tempBuffer), "[size: %i] [Single Cycle]",
                   filesize);
    } else {
      npf_snprintf(tempBuffer, sizeof(tempBuffer), "[size: %i]", filesize);
    }

    // Convert to etl::string for consistency
    etl::string<PFILENAME_SIZE> statusText = tempBuffer;

    x = 1;  // align with rest screen title & file list
    y = 23; // bottom line
    DrawString(x, y, statusText.c_str(), props);
  }

  SetColor(CD_NORMAL);
};

void ImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick){};

void ImportView::OnFocus() {
  auto fs = FileSystem::GetInstance();

  toInstr_ = viewData_->currentInstrumentID_;

  setCurrentFolder(fs, SAMPLES_LIB_DIR);
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
  // Get file size to check if it's a single cycle waveform
  auto fs = FileSystem::GetInstance();
  unsigned fileIndex = fileIndexList_[currentIndex_];
  int fileSize = fs->getFileSize(fileIndex);

  // check for LGPT or AKWF standard file sizes
  bool isSingleCycle = IS_SINGLE_CYCLE(fileSize);

  Trace::Debug("Preview: currentIndex_=%zu, previewPlayingIndex_=%zu, "
               "IsPlaying=%d, isSingleCycle=%d",
               currentIndex_, previewPlayingIndex_,
               Player::GetInstance()->IsPlaying(), isSingleCycle);

  // Check if we're trying to play the same sample that's already playing
  bool isSameSample = (currentIndex_ == previewPlayingIndex_);

  if (Player::GetInstance()->IsPlaying()) {
    // Always stop the current playback
    Player::GetInstance()->StopStreaming();

    if (isSameSample) {
      // If it's the same sample, just stop (toggle off)
      Trace::Debug("Stopping playback of current sample");
      previewPlayingIndex_ = (size_t)-1;
    } else {
      // If it's a different sample, start playing it
      Trace::Debug("Switching to play different sample");
      previewPlayingIndex_ = currentIndex_;

      // Use looping for single cycle waveforms
      if (isSingleCycle) {
        Trace::Debug("Looping single cycle waveform: %s (size: %d bytes)", name,
                     fileSize);
        Player::GetInstance()->StartLoopingStreaming(name);
      } else {
        Player::GetInstance()->StartStreaming(name);
      }
    }
  } else {
    // Nothing is playing, so start playing this sample
    Trace::Debug("Starting playback of sample");
    previewPlayingIndex_ = currentIndex_;

    // Use looping for single cycle waveforms
    if (isSingleCycle) {
      Trace::Debug("Looping single cycle waveform: %s (size: %d bytes)", name,
                   fileSize);
      Player::GetInstance()->StartLoopingStreaming(name);
    } else {
      Player::GetInstance()->StartStreaming(name);
    }
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

void ImportView::setCurrentFolder(FileSystem *fs, const char *name) {
  // Trace::Log("PICOIMPORT", "set Current Folder:%s");
  if (!fs->chdir(name)) {
    Trace::Error("FAILED to chdir to %s", name);
  }
  currentIndex_ = 0;
  // now update list of file indexes in this new dir
  fs->list(&fileIndexList_, ".wav", false);

  bool isSampleLibDir = fs->isParentRoot();
  if (isSampleLibDir && !fileIndexList_.empty()) {
    fileIndexList_.erase(fileIndexList_.begin());
  }
}
