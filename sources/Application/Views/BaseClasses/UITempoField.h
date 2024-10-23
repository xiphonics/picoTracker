#ifndef _UI_TEMPOFIELD_H_
#define _UI_TEMPOFIELD_H_

#include "Foundation/Observable.h"
#include "UIIntVarField.h"

class UITempoField : public UIIntVarField, public I_Observer {
public:
  UITempoField(FourCC action, GUIPoint &position, Variable &variable,
               const char *format, int min, int max, int xOffset, int yOffset);
  virtual void OnBClick();
  void Update(Observable &, I_ObservableData *);
  void ProcessArrow(unsigned short mask);
  void ProcessBArrow(unsigned short mask);

private:
  FourCC action_;
};
#endif
