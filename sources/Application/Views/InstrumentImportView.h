#ifndef _INSTRUMENT_IMPORT_VIEW_H_
#define _INSTRUMENT_IMPORT_VIEW_H_

#include "Foundation/T_SimpleList.h"
#include "ScreenView.h"
#include "System/FileSystem/PicoFileSystem.h"
#include "ViewData.h"
#include <string>

class InstrumentImportView : public ScreenView {
public:
  InstrumentImportView(GUIWindow &w, ViewData *viewData);
  ~InstrumentImportView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int tick = 0);
  virtual void OnFocus();

protected:
  void setCurrentFolder(PicoFileSystem *picoFS, const char *name);
  void warpToNextInstrument(bool goUp);
  void importInstrument(char *name);

private:
  size_t topIndex_ = 0;
  size_t currentIndex_ = 0;
  short selected_ = 0;
  int toInstrID_ = 0;
  etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList_;
};
#endif
