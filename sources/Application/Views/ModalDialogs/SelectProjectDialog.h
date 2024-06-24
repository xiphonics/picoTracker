#ifndef _SELECT_PROJECT_DIALOG_H_
#define _SELECT_PROJECT_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "System/Errors/Result.h"

class SelectProjectDialog : public ModalView {
public:
  SelectProjectDialog(View &view);
  ~SelectProjectDialog();

  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick);
  virtual void OnFocus();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void AnimationUpdate(){};

  Result OnNewProject(const char *name);

  const char *GetSelection() { return selection_; }

protected:
  void warpToNextProject(bool goUp);
  void setCurrentFolder();

private:
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  int currentProject_;
  int selected_;
  char selection_[MAX_PROJECT_NAME_LENGTH];
  static int lastProject_;
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};

#endif
