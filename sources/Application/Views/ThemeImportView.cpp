/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ThemeImportView.h"
#include "Application/AppWindow.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "ModalDialogs/MessageBox.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "ThemeView.h"
#include <nanoprintf.h>

#define LIST_WIDTH (SCREEN_WIDTH - 2)
// -4 to allow for title, filesize & spacers
#define LIST_PAGE_SIZE (SCREEN_HEIGHT - 4)

ThemeImportView::ThemeImportView(GUIWindow &w, ViewData *viewData)
    : ScreenView(w, viewData) {}

ThemeImportView::~ThemeImportView() {}

void ThemeImportView::Reset() {
  topIndex_ = 0;
  currentIndex_ = 0;
  fileIndexList_.clear();
}

void ThemeImportView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_PLAY) {
    auto fs = FileSystem::GetInstance();
    char name[PFILENAME_SIZE];

    if (currentIndex_ < fileIndexList_.size()) {
      unsigned fileIndex = fileIndexList_[currentIndex_];
      fs->getFileName(fileIndex, name, PFILENAME_SIZE);

      if (mask & EPBM_ALT) {
        Trace::Log("THEMEIMPORT", "SHIFT play - import");
        onImportTheme(name);
      }
    }

    // handle moving up and down the file list
  } else if (mask & EPBM_UP) {
    warpToNextTheme(true);
  } else if (mask & EPBM_DOWN) {
    warpToNextTheme(false);
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_NAV)) {
    // Go to back "left" to theme screen
    ViewType vt = VT_THEME;
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
        } else {
          // If it's a file, import it
          onImportTheme(name);
        }
      }
    }
  }
};

void ThemeImportView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  auto fs = FileSystem::GetInstance();

  // Draw title
  const char *title = "Import Theme";
  SetColor(CD_INFO);
  DrawString(pos._x + 1, pos._y, title, props);

  SetColor(CD_NORMAL);

  // Draw theme files
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
      // Remove the .ptt extension for display
      char *dot = strrchr(buffer, '.');
      if (dot) {
        *dot = '\0';
      }
      DrawString(x, y, buffer, props);
    }
    y++;
  }
};

void ThemeImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {}

void ThemeImportView::OnFocus() {
  auto fs = FileSystem::GetInstance();

  // Navigate to the themes directory
  setCurrentFolder(fs, THEMES_DIR);
}

void ThemeImportView::warpToNextTheme(bool goUp) {
  if (fileIndexList_.empty())
    return;

  if (goUp) {
    if (currentIndex_ > 0) {
      currentIndex_--;
      // if we have scrolled off the top, page the file list up if not
      // already at very top of the list
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

void ThemeImportView::onImportTheme(const char *filename) {
  AppWindow::AutoSaveBlockGuard autoSaveBlockGuard;

  // Use Config's ImportTheme method directly
  Config *config = Config::GetInstance();
  bool result = config->ImportTheme(filename);

  if (result) {
    // Theme was successfully imported and applied to config
    // Save the config to ensure changes persist
    config->Save();

    // Get the AppWindow to update colors
    AppWindow &app = (AppWindow &)w_;
    app.UpdateColorsFromConfig();

    // Show success message
    MessageBox *mb =
        MessageBox::Create(*this, "Theme imported successfully", MBBF_OK);
    DoModal(mb, [](View &v, ModalView &dialog) {
      if (dialog.GetReturnCode() == MBL_OK) {
        // Switch back to the theme view
        ViewType vt = VT_THEME;
        ViewEvent ve(VET_SWITCH_VIEW, &vt);
        v.SetChanged();
        v.NotifyObservers(&ve);
      }
    });
  } else {
    // Show error message
    MessageBox *mb =
        MessageBox::Create(*this, "Failed to import theme", MBBF_OK);
    DoModal(mb, [](View &v, ModalView &dialog) {
      if (dialog.GetReturnCode() == MBL_OK) {
        // Switch back to the theme view
        ViewType vt = VT_THEME;
        ViewEvent ve(VET_SWITCH_VIEW, &vt);
        v.SetChanged();
        v.NotifyObservers(&ve);
      }
    });
  }
  isDirty_ = true;
}

void ThemeImportView::setCurrentFolder(FileSystem *fs, const char *name) {
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
  fs->list(&fileIndexList_, THEME_FILE_EXTENSION, false);
  Trace::Debug("loaded %d files from %s", fileIndexList_.size(), name);
}
