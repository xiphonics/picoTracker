#ifndef _SELECTPROJECT_VIEW_H_
#define _SELECTPROJECT_VIEW_H_

#include "Foundation/T_SimpleList.h"
#include "ScreenView.h"
#include "System/FileSystem/FileSystem.h"
#include "ViewData.h"
#include <string>

class SelectProjectView : public ScreenView {
public:
  SelectProjectView(GUIWindow &w, ViewData *viewData);
  ~SelectProjectView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();
  void getSelectedProjectName(char *name);

protected:
  void setCurrentFolder();
  void warpToNextProject(bool goUp);

private:
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  char selection_[MAX_PROJECT_NAME_LENGTH];
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};
#endif
