#ifndef _PICO_IMPORT_SAMPLE_DIALOG_H_
#define _PICO_IMPORT_SAMPLE_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "Foundation/T_SimpleList.h"
#include "System/FileSystem/PicoFileSystem.h"
#include <string>

class PicoImportSampleDialog : public ModalView {
public:
  PicoImportSampleDialog(View &view);
  virtual ~PicoImportSampleDialog();

  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick);
  virtual void OnFocus();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void AnimationUpdate() {};

protected:
  void setCurrentFolder(Path *path);
  void warpToNextSample(int dir);
  void import(Path &element);
  void preview(Path &element);

private:
  short topIndex_;
  short currentIndex_;
  short selected_;
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};

#endif
