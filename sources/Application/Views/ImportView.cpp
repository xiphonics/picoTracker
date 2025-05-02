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
  // Check for key release events
  if (!pressed) {
    // Check if play key was released
    if (playKeyHeld_ && !(mask & EPBM_PLAY)) {
      // Play key no longer pressed so should stop playback
      playKeyHeld_ = false;
      if (Player::GetInstance()->IsPlaying()) {
        Player::GetInstance()->StopStreaming();
        previewPlayingIndex_ = (size_t)-1;
      }
      return;
    }

    // Check if edit key was released
    if (editKeyHeld_ && !(mask & EPBM_EDIT)) {
      // Edit key no longer pressed, redraw the view to clear status message
      editKeyHeld_ = false;
      isDirty_ = true; // Mark view as dirty to force redraw
      return;
    }
  }

  // Handle key press events
  if (pressed) {
    if (mask & EPBM_PLAY) {
      auto fs = FileSystem::GetInstance();
      char name[PFILENAME_SIZE];
      unsigned fileIndex = fileIndexList_[currentIndex_];
      fs->getFileName(fileIndex, name, PFILENAME_SIZE);

      // Set flag to track that play key is being held
      playKeyHeld_ = true;

      if (mask & EPBM_ALT) {
        Trace::Log("PICOIMPORT", "SHIFT play - import");
        import(name);
      } else {
        Trace::Log("PICOIMPORT", "play key pressed - start preview");
        preview(name);
      }
      return; // We've handled the play button, so return
    }

    // Handle EDIT+UP and EDIT+DOWN for preview volume adjustment
    if (mask & EPBM_EDIT) {
      // Set flag to track that edit key is being held
      editKeyHeld_ = true;

      if (mask & EPBM_UP) {
        // EDIT+UP: Increase preview volume
        adjustPreviewVolume(true);
        return;
      } else if (mask & EPBM_DOWN) {
        // EDIT+DOWN: Decrease preview volume
        adjustPreviewVolume(false);
        return;
      }
    }
  }

  // Only process other buttons when pressed
  if (!pressed)
    return;

  // handle moving up and down the file list
  if (mask & EPBM_UP) {
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

      displayName += tempBuffer;
      // Format the display name with appropriate prefix
      if (isSingleCycle) {
        SetColor(CD_INFO);
        DrawString(x, y, "~", props);
        SetColor(CD_NORMAL);
      } else {
        DrawString(x, y, " ", props);
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

    DrawString(x + 1, y, displayName.c_str(), props);
    y += 1;
  };

  // draw current selected file size, preview volume and single cycle indicator
  SetColor(CD_HILITE2);
  props.invert_ = true;
  y = 0;
  auto currentFileIndex = fileIndexList_[currentIndex_];
  if (fs->getFileType(currentFileIndex) == PFT_FILE) {
    int filesize = fs->getFileSize(currentFileIndex);
    // check for LGPT or AKWF standard file sizes
    bool isSingleCycle = IS_SINGLE_CYCLE(filesize);

    // Get the current preview volume
    int previewVolume = 0;
    Variable *v = viewData_->project_->FindVariable(FourCC::VarPreviewVolume);
    if (v) {
      previewVolume = v->GetInt();
    }

    // Create a temporary buffer for formatting
    char tempBuffer[PFILENAME_SIZE];

    if (isSingleCycle) {
      npf_snprintf(tempBuffer, sizeof(tempBuffer), "vol:%2d%% [size: %i] [1C]",
                   previewVolume, filesize);
    } else {
      npf_snprintf(tempBuffer, sizeof(tempBuffer), "vol:%2d%% [size: %i]",
                   previewVolume, filesize);
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

  // If something is already playing, stop it first
  if (Player::GetInstance()->IsPlaying()) {
    Player::GetInstance()->StopStreaming();
  }

  // Start playing the selected sample
  Trace::Debug("Starting preview of %s (single cycle: %d)", name,
               isSingleCycle);
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

void ImportView::import(char *name) {
  // stop playing before trying to import
  if (Player::GetInstance()->IsPlaying()) {
    MessageBox *mb =
        new MessageBox(*this, "Can't import while previewing", MBBF_OK);
    DoModal(mb);
    return;
  }

  // Get current project name
  char projName[MAX_PROJECT_NAME_LENGTH];
  viewData_->project_->GetProjectName(projName);

  // Check if we're in the project's sample directory
  if (inProjectSampleDir_) {
    MessageBox *mb =
        new MessageBox(*this, "Can't import from project!", MBBF_OK);
    DoModal(mb);
    return;
  }

  SamplePool *pool = SamplePool::GetInstance();

  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_start_blocking();
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

void ImportView::adjustPreviewVolume(bool increase) {
  // Get the project instance
  Project *project = viewData_->project_;

  // Find the preview volume variable
  Variable *v = project->FindVariable(FourCC::VarPreviewVolume);
  if (!v) {
    Status::Set("Preview volume setting not found");
    return;
  }

  // Get current value
  int currentVolume = v->GetInt();

  // Calculate new value (increase or decrease by 5)
  int step = 5;
  int newVolume = increase ? currentVolume + step : currentVolume - step;

  // Clamp to valid range (0-100)
  newVolume = newVolume > 100 ? 100 : newVolume;
  newVolume = newVolume < 0 ? 0 : newVolume;

  // Set the new value
  v->SetInt(newVolume);

  // Mark the view as dirty to update the status bar with the new volume
  isDirty_ = true;

  // If currently previewing, restart the preview to apply the new volume
  if (Player::GetInstance()->IsPlaying() &&
      previewPlayingIndex_ != (size_t)-1) {
    // Get the currently playing file name
    auto fs = FileSystem::GetInstance();
    char name[PFILENAME_SIZE];
    unsigned fileIndex = fileIndexList_[previewPlayingIndex_];
    fs->getFileName(fileIndex, name, PFILENAME_SIZE);

    // Restart the preview with the new volume
    preview(name);
  }
}

void ImportView::setCurrentFolder(FileSystem *fs, const char *name) {
  // Reset the project sample directory flag
  inProjectSampleDir_ = false;

  // Special case: if we're trying to go up (..) from a top-level directory
  if (strcmp(name, "..") == 0) {
    // Check if we're in a top-level directory (parent is root)
    if (fs->isParentRoot()) {
      Trace::Log("PICOIMPORT",
                 "Detected top-level directory, navigating to root");
      // Navigate directly to root instead of using ".."
      fs->chdir("/");
    }
  }

  // Normal directory navigation
  if (!fs->chdir(name)) {
    Trace::Error("FAILED to chdir to %s", name);
  }
  currentIndex_ = 0;
  // Update list of file indexes in this new dir
  fs->list(&fileIndexList_, ".wav", false);

  // Check if we're in the project's sample directory
  // This is a simple check - if we're in a directory called "samples"
  // and its parent is a directory with the same name as the current project
  char projName[MAX_PROJECT_NAME_LENGTH];
  viewData_->project_->GetProjectName(projName);

  if (strcmp(name, PROJECT_SAMPLES_DIR) == 0) {
    // We just navigated to a directory called "samples"
    // Check if its parent directory has the same name as the current project
    etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> expectedPath(PROJECTS_DIR);
    expectedPath.append("/");
    expectedPath.append(projName);

    // If we can navigate to this path from the root, and then to samples,
    // and that's where we are now, then we're in the project's sample directory
    inProjectSampleDir_ = true;
    Trace::Log("PICOIMPORT", "Now in project sample directory");
  } else if (strcmp(name, "..") == 0) {
    // We're navigating up, so we're no longer in the project's sample directory
    inProjectSampleDir_ = false;
  }
}
