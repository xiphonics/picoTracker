#include "InstrumentImportView.h"
#include "Application/AppWindow.h"
#include "Application/Instruments/I_Instrument.h"
#include "Application/Persistency/PersistencyService.h"
#include "ModalDialogs/MessageBox.h"
#include <memory>
#include <nanoprintf.h>

#define LIST_WIDTH (SCREEN_WIDTH - 2)
// -4 to allow for title, filesize & spacers
#define LIST_PAGE_SIZE (SCREEN_HEIGHT - 4)

InstrumentImportView::InstrumentImportView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {
  // Store the current instrument ID
  toInstrID_ = viewData_->currentInstrumentID_;
}

InstrumentImportView::~InstrumentImportView() {}

void InstrumentImportView::ProcessButtonMask(unsigned short mask,
                                             bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_PLAY) {
    auto fs = FileSystem::GetInstance();
    char name[PFILENAME_SIZE];

    if (currentIndex_ < fileIndexList_.size()) {
      unsigned fileIndex = fileIndexList_[currentIndex_];
      fs->getFileName(fileIndex, name, PFILENAME_SIZE);

      if (mask & EPBM_ALT) {
        Trace::Log("INSTRUMENTIMPORT", "SHIFT play - import");
        importInstrument(name);
      } else {
        // TODO: audition instrument by temporarily loading it and playing a
        // note
      }
    }

    // handle moving up and down the file list
  } else if (mask & EPBM_UP) {
    warpToNextInstrument(true);
  } else if (mask & EPBM_DOWN) {
    warpToNextInstrument(false);
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_NAV)) {
    // Go to back "left" to instrument screen
    ViewType vt = VT_INSTRUMENT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
  } else {
    // ENTER modifier
    if (mask & EPBM_ENTER) {
      auto fs = FileSystem::GetInstance();

      if (currentIndex_ < fileIndexList_.size()) {
        unsigned fileIndex = fileIndexList_[currentIndex_];
        char name[PFILENAME_SIZE];
        fs->getFileName(fileIndex, name, PFILENAME_SIZE);

        // Only allow navigation into directories, not to parent directory
        if (fs->getFileType(fileIndex) == PFT_DIR && strcmp(name, ".") != 0 &&
            strcmp(name, "..") != 0) {
          setCurrentFolder(fs, name);
          isDirty_ = true;
          topIndex_ = 0; // need to reset when entering a dir as prev dir may
                         // have been already scrolled down
        }
      }
    }
  }
};

void InstrumentImportView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  auto fs = FileSystem::GetInstance();

  // Draw title
  const char *title = "Import Instrument";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);

  SetColor(CD_NORMAL);

  // Draw instrument files
  int x = 1;
  int y = pos._y + 2;

  // need to use fullsize buffer as sdfat doesnt truncate if filename longer
  // than buffer but instead returns empty string in buffer :-(
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
    fs->getFileName(fileIndex, buffer, PFILENAME_SIZE);

    if (fs->getFileType(fileIndex) == PFT_DIR) {
      // Draw a directory
      DrawString(x, y, "[", props);
      DrawString(x + 1, y, buffer, props);
      DrawString(x + strlen(buffer) + 1, y, "]", props);
    } else {
      // Draw a file
      DrawString(x, y, buffer, props);
    }
    y++;
  }
};

void InstrumentImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {}

void InstrumentImportView::OnFocus() {
  auto fs = FileSystem::GetInstance();

  // Navigate to the instruments directory
  setCurrentFolder(fs, INSTRUMENTS_DIR);
}

void InstrumentImportView::warpToNextInstrument(bool goUp) {
  if (fileIndexList_.empty())
    return;

  if (goUp) {
    if (currentIndex_ > 0) {
      currentIndex_--;
      if (currentIndex_ < topIndex_) {
        topIndex_ = currentIndex_;
      }
    }
  } else {
    if (currentIndex_ < fileIndexList_.size() - 1) {
      currentIndex_++;
      if (currentIndex_ >= topIndex_ + LIST_PAGE_SIZE) {
        topIndex_ = currentIndex_ - LIST_PAGE_SIZE + 1;
      }
    }
  }
  isDirty_ = true;
}

void InstrumentImportView::importInstrument(char *name) {
  // TODO: check that current instrument is not modified

  // Get the current instrument bank
  InstrumentBank *bank = viewData_->project_->GetInstrumentBank();

  // assert bank is not null
  assert(bank);

  // use current instrument ID if importing a new instrument
  toInstrID_ = viewData_->currentInstrumentID_;

  // First detect the instrument type from the file
  InstrumentType importedType =
      PersistencyService::GetInstance()->DetectInstrumentType(name);

  if (importedType == IT_NONE) {
    MessageBox *mb = new MessageBox(*this, "Unknown instrument type", MBBF_OK);
    DoModal(mb);
    return;
  }

  Trace::Log("INSTRUMENTIMPORT", "Detected instrument type: %d", importedType);
  Trace::Log("INSTRUMENTIMPORT", "Importing instrument from file: %s", name);
  Trace::Log("INSTRUMENTIMPORT", "Target instrument ID: %d", toInstrID_);

  // Get the current instrument
  I_Instrument *currentInstrument = bank->GetInstrument(toInstrID_);

  if (!currentInstrument) {
    MessageBox *mb = new MessageBox(*this, "Invalid instrument", MBBF_OK);
    DoModal(mb);
    return;
  }

  // Log the current instrument type for debugging
  Trace::Log("INSTRUMENTIMPORT",
             "Current instrument type: %d, Imported type: %d",
             currentInstrument->GetType(), importedType);

  if (currentInstrument->GetType() != importedType) {
    Trace::Log("INSTRUMENTIMPORT", "Converting instrument from type %d to %d",
               currentInstrument->GetType(), importedType);

    // Delete the current instrument
    bank->releaseInstrument(toInstrID_);

    // Create a new instrument of the correct type in the same slot
    if (bank->GetNextAndAssignID(importedType, toInstrID_) ==
        NO_MORE_INSTRUMENT) {
      MessageBox *mb =
          new MessageBox(*this, "Failed to create instrument", MBBF_OK);
      DoModal(mb);
      return;
    }

    Trace::Log("INSTRUMENTIMPORT",
               "Created new instrument of type %d in slot %d", importedType,
               toInstrID_);
  } else {
    Trace::Log("INSTRUMENTIMPORT",
               "Keeping existing instrument of type %d in slot %d",
               importedType, toInstrID_);
  }

  // Force the ViewData to update its current instrument type
  // This ensures the InstrumentView will show the correct instrument type
  // when we return to it
  Trace::Log("INSTRUMENTIMPORT", "Created new instrument of type %d",
             importedType);

  // Get the updated instrument (which may be a new one if we converted)
  I_Instrument *instrument = bank->GetInstrument(toInstrID_);

  // Import the instrument settings
  PersistencyResult result =
      PersistencyService::GetInstance()->ImportInstrument(instrument, name);

  // debug logging to show final instrument type
  Trace::Log("INSTRUMENTIMPORT", "Imported TYPE: %d", instrument->GetType());

  if (result == PERSIST_LOADED) {
    // Force a strong notification to ensure the UI updates
    // First mark the instrument as changed
    instrument->SetChanged();

    // Then notify all observers to update their state
    instrument->NotifyObservers();

    // Log the final state of the instrument for debugging
    Variable *channelVar =
        instrument->FindVariable(FourCC::MidiInstrumentChannel);
    if (channelVar) {
      Trace::Log("INSTRUMENTIMPORT", "Final MIDI channel: %d",
                 channelVar->GetInt());
    }

    // Log the final instrument type for debugging
    Trace::Log("INSTRUMENTIMPORT",
               "Final instrument type in importInstrument: %d",
               instrument->GetType());

    // Update the current instrument ID in the view data
    // This ensures the InstrumentView will display the correct instrument when
    // we return
    viewData_->currentInstrumentID_ = toInstrID_;
    Trace::Log("INSTRUMENTIMPORT",
               "Updated viewData_ currentInstrumentID_ to: %d", toInstrID_);

    // Show success message and return to instrument view
    MessageBox *mb = new MessageBox(*this, "Import successful", MBBF_OK);
    DoModal(mb, [this, instrument](View &v, ModalView &dialog) {
      if (dialog.GetReturnCode() == MBL_OK) {
        // Log that we're about to switch back to the instrument view
        Trace::Log("INSTRUMENTIMPORT",
                   "Switching back to instrument view with ID: %d", toInstrID_);

        // Force another notification to ensure the UI updates when we return
        instrument->SetChanged();
        instrument->NotifyObservers();

        // Switch back to the instrument view
        ViewType vt = VT_INSTRUMENT;
        ViewEvent ve(VET_SWITCH_VIEW, &vt);
        v.SetChanged();
        v.NotifyObservers(&ve);
      }
    });
  } else {
    MessageBox *mb = new MessageBox(*this, "Import failed", MBBF_OK);
    DoModal(mb);
  }

  isDirty_ = true;
}

void InstrumentImportView::setCurrentFolder(FileSystem *fs, const char *name) {
  if (!fs->chdir(name)) {
    Trace::Error("FAILED to chdir to %s", name);
    return;
  }
  currentIndex_ = 0;
  topIndex_ = 0;
  isDirty_ = true;

  // Update list of file indexes in this new dir
  fileIndexList_.clear();
  // Use false for subDirOnly to include both files and directories
  fs->list(&fileIndexList_, INSTRUMENT_FILE_EXTENSION, false);
  Trace::Debug("loaded %d files from %s", fileIndexList_.size(), name);
}
