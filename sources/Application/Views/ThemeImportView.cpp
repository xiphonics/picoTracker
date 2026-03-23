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
#include "Foundation/Services/MemoryService.h"
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

void ThemeImportView::OpenSelectedItem() {
  if (currentIndex_ >= fileIndexList_.size())
    return;

  // get selected item
  int fileIndex = fileIndexList_[currentIndex_];
  auto fs = FileSystem::GetInstance();
  char name[PFILENAME_SIZE];
  fs->getFileName(fileIndex, name, PFILENAME_SIZE);
  onImportTheme(name);
}

void ThemeImportView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  if (mask & EPBM_ENTER) {
    OpenSelectedItem();
  } else if (mask & EPBM_UP) {
    changeSelection(mask & EPBM_EDIT ? -LIST_PAGE_SIZE : -1);
  } else if (mask & EPBM_DOWN) {
    changeSelection(mask & EPBM_EDIT ? LIST_PAGE_SIZE : 1);
  } else if ((mask & EPBM_LEFT) && (mask & EPBM_NAV)) {
    // Go to back "left" to theme screen
    ViewType vt = VT_THEME;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
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

  // Draw theme files
  int x = 1;
  int y = pos._y + 2;

  // need to use fullsize buffer as sdfat doesnt truncate if filename longer
  // than buffer but instead returns empty string in buffer :-(
  size_t total = fileIndexList_.size();
  char buffer[PFILENAME_SIZE];
  for (size_t i = topIndex_; i < topIndex_ + LIST_PAGE_SIZE && (i < total);
       i++) {
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

    // Remove the .ptt extension for display
    char *dot = strrchr(buffer, '.');
    if (dot) {
      *dot = '\0';
    }
    DrawString(x, y, buffer, props);

    y++;
  }

  drawScrollBar(SCREEN_WIDTH - 1, pos._y + 2, LIST_PAGE_SIZE, topIndex_, total);
};

void ThemeImportView::OnPlayerUpdate(PlayerEventType, unsigned int tick) {}

void ThemeImportView::OnFocus() {
  // Navigate to the themes directory
  setCurrentFolder();
}

void ThemeImportView::changeSelection(int delta) {
  if (fileIndexList_.empty())
    return;

  size_t max = fileIndexList_.size() - 1;
  size_t target = std::clamp((int)(currentIndex_ + delta), 0, (int)max);
  size_t bottom = std::min(topIndex_ + LIST_PAGE_SIZE - 1, max);
  size_t landingOn = std::clamp(target, topIndex_, bottom);
  bool willScroll = (currentIndex_ == landingOn) && (landingOn != target);

  currentIndex_ = willScroll ? target : landingOn;

  if (willScroll) {
    size_t offset = (target > bottom) ? LIST_PAGE_SIZE - 1 : 0;
    size_t upper = fileIndexList_.size() > LIST_PAGE_SIZE
                       ? fileIndexList_.size() - LIST_PAGE_SIZE
                       : 0zu;
    size_t base = target > offset ? target - offset : 0zu;
    topIndex_ = std::min(base, upper);
  }

  isDirty_ = true;
}

void ThemeImportView::onImportTheme(const char *filename) {
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

    // make sure we redraw everything with the new colors
    ForceClear();

    // Show success message
    MessageBox *mb =
        MessageBox::Create(*this, "Theme imported successfully", MBBF_OK);
    DoModal(
        mb,
        ModalViewCallback::create<ThemeImportView,
                                  &ThemeImportView::onImportThemeModalDismiss>(
            *this));
  } else {
    // Show error message
    MessageBox *mb =
        MessageBox::Create(*this, "Failed to import theme", MBBF_OK);
    DoModal(
        mb,
        ModalViewCallback::create<ThemeImportView,
                                  &ThemeImportView::onImportThemeModalDismiss>(
            *this));
  }
  isDirty_ = true;
}

void ThemeImportView::onImportThemeModalDismiss(View &, ModalView &dialog) {
  if (dialog.GetReturnCode() != MBL_OK) {
    return;
  }

  ViewType vt = VT_THEME;
  ViewEvent ve(VET_SWITCH_VIEW, &vt);
  SetChanged();
  NotifyObservers(&ve);
}

void ThemeImportView::setCurrentFolder() {
  auto fs = FileSystem::GetInstance();

  if (!fs->chdir(THEMES_DIR)) {
    Trace::Error("FAILED to chdir to %s", THEMES_DIR);
    return;
  }

  currentIndex_ = 0;
  topIndex_ = 0;
  isDirty_ = true;

  // get the directory listing
  fs->list(&fileIndexList_, THEME_FILE_EXTENSION, false, false, true);

  // remove directories from listing
  for (int i = fileIndexList_.size() - 1; i >= 0; i--) {
    if (fs->getFileType(fileIndexList_[i]) == PFT_DIR) {
      fileIndexList_.erase(fileIndexList_.begin() + i);
    }
  }

  Trace::Debug("loaded %d files from %s", fileIndexList_.size(), THEMES_DIR);
}
