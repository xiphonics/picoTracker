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
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();

protected:
  void setCurrentFolder(FileSystem *fs, const char *name);
  void warpToNextSample(bool goUp);
  void import(char *name);
  void preview(char *name);

private:
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  size_t previewPlayingIndex_ = 0;
  short selected_ = 0;
  int toInstr_ = 0;
  bool playKeyHeld_ =
      false; // Flag to track when the play key is being held down
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};
#endif
