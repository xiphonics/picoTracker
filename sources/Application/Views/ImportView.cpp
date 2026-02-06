/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ImportView.h"
#include "Application/Audio/AudioFileStreamer.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Views/SampleEditorView.h"
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/to_string.h"
#include "ModalDialogs/MessageBox.h"
#include "System/FileSystem/FileSystem.h"
#include "ViewUtils.h"
#include <memory>
#include <nanoprintf.h>

#define LIST_WIDTH SCREEN_WIDTH - 2
// -4 to allow for title, filesize & spacers
#define LIST_PAGE_SIZE SCREEN_HEIGHT - 5

// is single cycle macro, checks for FILE size of LGPT and AKWF file formats
// AKWF "nes" pack 1376, AKWF "standard" 1344, LGPT pack 300
#define IS_SINGLE_CYCLE(x) (x == 1376 || x == 1344 || x == 300)

// Initialize static member
ViewType ImportView::sourceViewType_ = VT_PROJECT;

ImportView::ImportView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {}

ImportView::~ImportView() {}

void ImportView::Reset() {
  topIndex_ = 0;
  currentIndex_ = 0;
  previewPlayingIndex_ = 0;
  selectedButton_ = 0;
  toInstr_ = 0;
  playKeyHeld_ = false;
  editKeyHeld_ = false;
  inProjectSampleDir_ = false;
  fileIndexList_.clear();
}

// Static method to set the source view type before opening ImportView
void ImportView::SetSourceViewType(ViewType vt) { sourceViewType_ = vt; }

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
    auto fs = FileSystem::GetInstance();
    unsigned fileIndex = fileIndexList_[currentIndex_];

    if (mask & EPBM_PLAY) {
      char name[PFILENAME_SIZE];
      fs->getFileName(fileIndex, name, PFILENAME_SIZE);

      // Set flag to track that play key is being held
      playKeyHeld_ = true;

      if (mask & EPBM_ALT) {
        Trace::Log("PICOIMPORT", "SHIFT play - import");
        import();
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

    if (mask & EPBM_NAV && mask & EPBM_EDIT) {
      // toggle from sdcard "import sample" & project pool listing
      if (inProjectSampleDir_) {
        inProjectSampleDir_ = false;
        setCurrentFolder(fs, SAMPLES_LIB_DIR);
      } else {
        inProjectSampleDir_ = true;
        setCurrentFolder(fs, PROJECT_SAMPLES_DIR);
      }
    }

    if (mask & EPBM_ENTER) {
      if (inProjectSampleDir_) {
        if (fileIndexList_.empty()) {
          return; // Do nothing if the list is empty
        }
        // NOTE: the order of buttons in project pool is: edit, remove
        // while in file browser its: import, edit
        if (selectedButton_ == 0) {
          char name[PFILENAME_SIZE];
          fs->getFileName(fileIndex, name, PFILENAME_SIZE);
          showSampleEditor(name, false);
        } else {
#ifdef ADV
          removeProjectSample(fileIndex, fs);
#endif
        }
        return;
      }
      // we can't import or edit dirs!
      if (fs->getFileType(fileIndex) != PFT_DIR) {
        if (selectedButton_ == 0) {
          import();
        } else {
          char name[PFILENAME_SIZE];
          fs->getFileName(fileIndex, name, PFILENAME_SIZE);
          showSampleEditor(name, false);
        }
      }
    }

    // handle changing selected "bottom button", note: ignore if this is a
    // nav+arrow combo
    if ((mask & EPBM_LEFT || mask & EPBM_RIGHT) && !(mask & EPBM_NAV)) {
      if (inProjectSampleDir_ && fileIndexList_.empty()) {
        return; // Do nothing if the list is empty
      }
      // toggle the selected button
      selectedButton_ = (selectedButton_ == 0) ? 1 : 0;
      DrawView();
    }
  }

  // Only process other buttons when pressed
  if (!pressed)
    return;

  // handle moving up and down the file list
  if (mask & EPBM_UP) {
    if (inProjectSampleDir_ && fileIndexList_.empty()) {
      return; // Do nothing if the list is empty
    }
    warpToNextSample(true);
  } else if (mask & EPBM_DOWN) {
    if (inProjectSampleDir_ && fileIndexList_.empty()) {
      return; // Do nothing if the list is empty
    }
    warpToNextSample(false);
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_NAV)) {
    // clear this flag on leaving this screen
    viewData_->isShowingSampleEditorProjectPool = false;

    // Go back to the source view that opened the ImportView
    ViewEvent ve(VET_SWITCH_VIEW, &sourceViewType_);
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

  // Draw title with available storage space
  const char *baseTitle =
      inProjectSampleDir_ ? "Project Pool" : "Import Sample";

  // Create title with storage info
  char titleBuffer[40];
  npf_snprintf(titleBuffer, sizeof(titleBuffer), "%s", baseTitle);

  SetColor(CD_NORMAL);
  DrawString(pos._x + 1, pos._y, titleBuffer, props);

  // Draw samples
  int x = 1;
  int y = pos._y + 2;

  uint32_t availableSpace =
      SamplePool::GetInstance()->GetAvailableSampleStorageSpace();

  // Loop through visible files in the list
  for (size_t i = topIndex_;
       i < topIndex_ + LIST_PAGE_SIZE && (i < fileIndexList_.size()); i++) {
    props.invert_ = false;

    unsigned fileIndex = fileIndexList_[i];
    etl::string<PFILENAME_SIZE> displayName;

    if (fs->getFileType(fileIndex) != PFT_DIR) {
      SetColor(CD_NORMAL);
      // Handle regular files
      char tempBuffer[PFILENAME_SIZE];
      fs->getFileName(fileIndex, tempBuffer, PFILENAME_SIZE);

      // Check if it's a single cycle waveform
      int filesize = fs->getFileSize(fileIndex);
      bool isSingleCycle = IS_SINGLE_CYCLE(filesize);

      displayName += tempBuffer;
      // Format the display name with appropriate prefix
      if (inProjectSampleDir_ &&
          viewData_->project_->SampleInUse(
              etl::string<MAX_INSTRUMENT_FILENAME_LENGTH>(tempBuffer))) {
        SetColor(CD_ACCENT);
        DrawString(x, y, "*", props);
        SetColor(CD_NORMAL);
      } else if (isSingleCycle) {
        SetColor(CD_ACCENT);
        DrawString(x, y, "~", props);
        SetColor(CD_NORMAL);
      } else {
        DrawString(x, y, " ", props);
      }
    } else {
      SetColor(CD_ACCENT);
      // Handle directories
      char tempBuffer[PFILENAME_SIZE];
      displayName = "/";
      // clear temp buffer
      memset(tempBuffer, 0, PFILENAME_SIZE);
      fs->getFileName(fileIndex, tempBuffer, PFILENAME_SIZE);
      displayName += tempBuffer;
    }

    // Truncate to fit display width
    if (displayName.size() > LIST_WIDTH) {
      displayName.resize(LIST_WIDTH);
    }

    if (i == currentIndex_) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    }
    DrawString(x + 1, y, displayName.c_str(), props);
    y += 1;
  };

  SetColor(CD_HILITE1);
  y = SCREEN_HEIGHT - 2;
  if (!inProjectSampleDir_) {
    if (selectedButton_ == 0) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_HILITE1);
      props.invert_ = false;
    }
    DrawString(x, y, "Import", props);
    if (selectedButton_ == 1) {
      SetColor(CD_HILITE2);
      props.invert_ = true;
    } else {
      SetColor(CD_HILITE1);
      props.invert_ = false;
    }
    DrawString(x + 10, y, "Edit", props);
  } else {
    if (fileIndexList_.empty()) {
      // draw this a few lines down from *top* of screen
      SetColor(CD_NORMAL);
      props.invert_ = false;
      DrawString(2, 3, "[pool empty]", props);
    } else {
      // we make edit the first button to make things easier because remove is
      // only available for now on the Advance and even on Advance we dont want
      // remove to be the default button
      if (selectedButton_ == 0) {
        SetColor(CD_HILITE2);
        props.invert_ = true;
      } else {
        SetColor(CD_HILITE1);
        props.invert_ = false;
      }
      DrawString(x, y, "Edit", props);
      if (selectedButton_ == 1) {
        SetColor(CD_HILITE2);
        props.invert_ = true;
      } else {
        SetColor(CD_HILITE1);
        props.invert_ = false;
      }
#ifdef ADV
      DrawString(x + 10, y, "Remove", props);
#else
      DrawString(x + 10, y, "N/A", props);
#endif
    }
  }
  props.invert_ = false;
  y += 1;

  // draw current selected file size, preview volume and single cycle indicator
  SetColor(CD_NORMAL);
  props.invert_ = true;
  y = 0;
  uint32_t filesize = 0;
  auto currentFileIndex = fileIndexList_[currentIndex_];

  // only get file size if it's a file not a dir
  if (fs->getFileType(currentFileIndex) == PFT_FILE) {
    filesize = fs->getFileSize(currentFileIndex);
    // if file size is larger than available space, set color to warning
    if (filesize > availableSpace) {
      SetColor(CD_WARN);
    }
  }

  // Get the current preview volume
  int previewVolume = 0;
  Variable *v = viewData_->project_->FindVariable(FourCC::VarPreviewVolume);
  if (v) {
    previewVolume = v->GetInt();
  }

  // Create a temporary buffer for formatting
  char tempBuffer[SCREEN_WIDTH];
  tempBuffer[SCREEN_WIDTH - 1] = '\0';

  npf_snprintf(tempBuffer, sizeof(tempBuffer), "vol:%2d%% size:%i/%i",
               previewVolume, filesize, availableSpace);

  // pad status line buffer with trailing space chars to ensure the invert
  // color is applied to entire line
  npf_snprintf(tempBuffer, sizeof(tempBuffer), "%s%*s", tempBuffer,
               SCREEN_WIDTH - strlen(tempBuffer), " ");

  x = 1;  // align with rest screen title & file list
  y = 23; // bottom line
  DrawString(x, y, tempBuffer, props);

  SetColor(CD_NORMAL);
};

void ImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick){};

void ImportView::OnFocus() {
  auto fs = FileSystem::GetInstance();

  toInstr_ = viewData_->currentInstrumentID_;

  inProjectSampleDir_ = viewData_->isShowingSampleEditorProjectPool;

  if (inProjectSampleDir_) {
    goProjectSamplesDir(viewData_);
    setCurrentFolder(fs, ".");
  } else {
    setCurrentFolder(fs, viewData_->importViewStartDir);
  }
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
  auto fs = FileSystem::GetInstance();
  unsigned fileIndex = fileIndexList_[currentIndex_];

  // do not preview directories
  if (fs->getFileType(fileIndex) == PFT_DIR) {
    return;
  }

  // Get file size to check if it's a single cycle waveform
  int fileSize = fs->getFileSize(fileIndex);

  // check for LGPT or AKWF standard file sizes
  bool isSingleCycle = IS_SINGLE_CYCLE(fileSize);

  // If something is already playing, stop it first
  if (Player::GetInstance()->IsPlaying()) {
    Player::GetInstance()->StopStreaming();
  }

  WavFile wav;
  auto wavRes = wav.Open(name);
  MessageBox *mb = nullptr;
  if (!wavRes) {
    auto error = wavRes.error();
    switch (error) {
    case INVALID_FILE:
      mb = MessageBox::Create(*this, "Preview Failed", "Could not open file",
                              MBBF_OK);
      break;
    case UNSUPPORTED_FILE_FORMAT:
    case INVALID_HEADER:
    case UNSUPPORTED_WAV_FORMAT:
      mb = MessageBox::Create(*this, "Preview Failed", "Invalid file", MBBF_OK);
      break;
    case UNSUPPORTED_AUDIO_FORMAT:
    case UNSUPPORTED_BITDEPTH:
    case UNSUPPORTED_SAMPLERATE:
      mb = MessageBox::Create(*this, "Preview Failed", "Unsupported format",
                              MBBF_OK);
      break;
    }
  } else {
    wav.Close();
  }

  if (mb != nullptr) {
    DoModal(mb);
    return;
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

void ImportView::import() {
  // stop playing before trying to import
  if (Player::GetInstance()->IsPlaying()) {
    MessageBox *mb =
        MessageBox::Create(*this, "Can't import while", "previewing", MBBF_OK);
    DoModal(mb);
    return;
  }

  auto fs = FileSystem::GetInstance();
  char name[PFILENAME_SIZE];
  unsigned fileIndex = fileIndexList_[currentIndex_];
  fs->getFileName(fileIndex, name, PFILENAME_SIZE);

  // Get current project name
  char projName[MAX_PROJECT_NAME_LENGTH + 1];
  viewData_->project_->GetProjectName(projName);

  // Check if we're in the project's sample directory
  if (inProjectSampleDir_) {
    MessageBox *mb =
        MessageBox::Create(*this, "Can't import from project!", MBBF_OK);
    DoModal(mb);
    return;
  }

  SamplePool *pool = SamplePool::GetInstance();

  // Check if we've reached the maximum number of samples
  int currentCount = pool->GetNameListSize();
  if (currentCount >= MAX_SAMPLES) {
    // Show error dialog to inform the user
    char message[SCREEN_WIDTH];
    npf_snprintf(message, sizeof(message), "Limit of %d sample reached",
                 MAX_SAMPLES);
    MessageBox *mb =
        MessageBox::Create(*this, "Cannot Import Sample", message, MBBF_OK);
    DoModal(mb);
    return;
  }

  // Check if the sample would exceed available flash storage
  int fileSize = fs->getFileSize(fileIndex);

  // Check if the sample would fit in available storage
  if (!pool->CheckSampleFits(fileSize)) {
    // Get available flash space for the message
    uint32_t availableFlash =
        SamplePool::GetInstance()->GetAvailableSampleStorageSpace();

    // Show error dialog to inform the user
    char message[SCREEN_WIDTH];

    uint32_t availBytes = availableFlash;
    npf_snprintf(message, sizeof(message), "Only %d bytes free", availBytes);
    MessageBox *mb =
        MessageBox::Create(*this, "Sample Too Large", message, MBBF_OK);
    DoModal(mb);
    return;
  }

  // Check if wave file is in a supported format
  WavFile wav;
  auto wavRes = wav.Open(name);
  MessageBox *mb = nullptr;
  if (!wavRes) {
    auto error = wavRes.error();
    switch (error) {
    case INVALID_FILE:
      mb = MessageBox::Create(*this, "Import Failed", "Could not open file",
                              MBBF_OK);
      break;
    case UNSUPPORTED_FILE_FORMAT:
    case INVALID_HEADER:
    case UNSUPPORTED_WAV_FORMAT:
      mb = MessageBox::Create(*this, "Import Failed", "invalid file", MBBF_OK);
      break;
    case UNSUPPORTED_AUDIO_FORMAT:
    case UNSUPPORTED_BITDEPTH:
    case UNSUPPORTED_SAMPLERATE:
      mb = MessageBox::Create(*this, "Import Failed", "unsupported format",
                              MBBF_OK);
      break;
    }
  } else {
    wav.Close();
  }

  if (mb != nullptr) {
    DoModal(mb);
    return;
  }

  int sampleID = pool->ImportSample(name, projName);

  if (sampleID >= 0) {
    I_Instrument *instr =
        viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
    if (instr->GetType() == IT_SAMPLE) {
      SampleInstrument *sinstr = (SampleInstrument *)instr;
      sinstr->AssignSample(sampleID);
      sinstr->ClearSlices();
    };

    // check if we had to truncate filename
    size_t nameLength = strlen(name);
    if (nameLength > MAX_INSTRUMENT_FILENAME_LENGTH) {
      Trace::Log("PICOIMPORT", "Filename too long: %s (%zu chars, max is %d)",
                 name, nameLength, MAX_INSTRUMENT_FILENAME_LENGTH);
      MessageBox *mb = MessageBox::Create(*this, "Sample name Truncated!",
                                          "Max filename length:24", MBBF_OK);
      DoModal(mb);
    }
  } else {
    Trace::Error("failed to import sample");
    // Show a generic error message if import failed for other reasons
    MessageBox *mb = MessageBox::Create(*this, "Import Failed",
                                        "Could not import sample", MBBF_OK);
    DoModal(mb);
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
}

void ImportView::setCurrentFolder(FileSystem *fs, const char *name) {
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
  refreshFileIndexList(fs);

  // Check if we're in the projects directory
  // and if trying to go into the same dir as current project and if so dont
  // allow it
  char projName[MAX_PROJECT_NAME_LENGTH + 1];
  viewData_->project_->GetProjectName(projName);

  if (strcmp(projName, name) == 0) {
    // We just navigated to a current project directory, not allowed!
    etl::string<MAX_PROJECT_SAMPLE_PATH_LENGTH> expectedPath(PROJECTS_DIR);
    // so instead go back out into the projects dir
    setCurrentFolder(fs, PROJECTS_DIR);

    Trace::Log("PICOIMPORT",
               "NOT allowed to browse into current project sample directory");
  }
}

void ImportView::showSampleEditor(
    etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename,
    bool isProjectSample) {

  viewData_->sampleEditorFilename = filename;

  // before going to sample editor set this view as its "source" view
  SampleEditorView::sourceViewType_ = VT_IMPORT;

  // Switch to the SampleEditorView
  ViewType vt = VT_SAMPLE_EDITOR;
  ViewEvent ve(VET_SWITCH_VIEW, &vt);
  SetChanged();
  NotifyObservers(&ve);
}

void ImportView::removeProjectSample(uint8_t fileIndex, FileSystem *fs) {
  char filename[PFILENAME_SIZE];
  fs->getFileName(fileIndex, filename, PFILENAME_SIZE);

  // first check if a instrument uses this sample
  bool inUse = viewData_->project_->SampleInUse(
      etl::string<MAX_INSTRUMENT_FILENAME_LENGTH>(filename));

  if (inUse) {
    MessageBox *mb =
        MessageBox::Create(*this, "Cannot remove", "Sample in use!", MBBF_OK);
    DoModal(mb);
    return;
  }

  // add spacing for basic way to size dialog wider to give Ok/cancel
  // buttons between space
  MessageBox *mb = MessageBox::Create(*this, "Remove sample?", filename,
                                      MBBF_OK | MBBF_CANCEL);
  DoModal(mb, [this, fs, filename, fileIndex](View &v, ModalView &dialog) {
    if (dialog.GetReturnCode() == MBL_OK) {
      // Translate filename to current sample pool index to avoid mismatches
      int sampleIndex =
          SamplePool::GetInstance()->FindSampleIndexByName(filename);
      if (sampleIndex < 0) {
        Trace::Error("Failed to map sample %s to pool index", filename);
        return;
      }
      // delete file
      if (!fs->DeleteFile(filename)) {
        Trace::Error("Failed to delete sample %s", filename);
        return;
      }
      // and unload it from ram
      SamplePool::GetInstance()->unloadSample(sampleIndex);

      if (currentIndex_ > 0) {
        --currentIndex_;
      }

      // refresh directory listing to avoid stale indexes
      refreshFileIndexList(fs);

      isDirty_ = true;
    }
  });
}

void ImportView::refreshFileIndexList(FileSystem *fs) {
  fs->list(&fileIndexList_, ".wav", false);

  if (fs->isCurrentRoot() || inProjectSampleDir_) {
    for (auto it = fileIndexList_.begin(); it != fileIndexList_.end(); ++it) {
      char entryName[PFILENAME_SIZE];
      fs->getFileName(*it, entryName, PFILENAME_SIZE);
      if (strcmp(entryName, "..") == 0) {
        fileIndexList_.erase(it);
        break;
      }
    }
  }

  if (currentIndex_ >= fileIndexList_.size()) {
    currentIndex_ = fileIndexList_.empty() ? 0 : fileIndexList_.size() - 1;
  }
}
