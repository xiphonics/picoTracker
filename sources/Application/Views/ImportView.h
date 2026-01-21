/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _IMPORT_VIEW_H_
#define _IMPORT_VIEW_H_

#include "Application/Views/ScreenView.h"
#include "Externals/etl/include/etl/vector.h"
#include "System/FileSystem/FileSystem.h"
#include "ViewData.h"
#include <string>

class ImportView : public ScreenView {
public:
  ImportView(GUIWindow &w, ViewData *viewData);
  ~ImportView();
  void Reset();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();

  // Static method to set which view will open the ImportView
  static void SetSourceViewType(ViewType vt);

  // Track which view opened the ImportView (default to project view)
  static ViewType sourceViewType_;

protected:
  void setCurrentFolder(FileSystem *fs, const char *name);
  void warpToNextSample(bool goUp);
  void import();
  void preview(char *name);
  void adjustPreviewVolume(bool increase);
  void showSampleEditor(etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> filename,
                        bool isProjectSample);
  void removeProjectSample(uint8_t fileIndex, FileSystem *fs);
  void refreshFileIndexList(FileSystem *fs);

private:
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  size_t previewPlayingIndex_ = 0;
  short selectedButton_ = 0;
  int toInstr_ = 0;
  bool playKeyHeld_ =
      false; // Flag to track when the play key is being held down
  bool editKeyHeld_ =
      false; // Flag to track when the edit key is being held down
  bool inProjectSampleDir_ =
      false; // Flag to track if we're in the project's sample directory
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};
#endif
