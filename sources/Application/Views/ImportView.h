#ifndef _IMPORT_VIEW_H_
#define _IMPORT_VIEW_H_

#include "BaseClasses/View.h"
#include "Foundation/T_SimpleList.h"
#include "System/FileSystem/PicoFileSystem.h"
#include "ViewData.h"
#include <string>

class ImportView : public View {
public:
  ImportView(GUIWindow &w, ViewData *viewData);
  ~ImportView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  virtual void AnimationUpdate();

protected:
  void setCurrentFolder(PicoFileSystem *picoFS, const char *name);
  void warpToNextSample(bool goUp);
  void import(char *name);
  void preview(char *name);

private:
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  size_t previewPlayingIndex_ = 0;
  short selected_ = 0;
  int toInstr_ = 0;
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};
#endif
