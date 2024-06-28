#ifndef _PICO_IMPORT_SAMPLE_DIALOG_H_
#define _PICO_IMPORT_SAMPLE_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "Foundation/T_SimpleList.h"
#include "System/FileSystem/PicoFileSystem.h"
#include <string>

#define SAMPLE_LIB "/samplelib"

class PicoImportSampleDialog : public ModalView {
public:
  PicoImportSampleDialog(View &view);
  virtual ~PicoImportSampleDialog();

  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick);
  virtual void OnFocus();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void AnimationUpdate(){};

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
