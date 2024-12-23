#ifndef _INSTRUMENT_VIEW_H_
#define _INSTRUMENT_VIEW_H_

#include "BaseClasses/FieldView.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIBitmaskVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIIntVarOffField.h"
#include "BaseClasses/UINoteVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Externals/etl/include/etl/vector.h"
#include "Foundation/Observable.h"
#include "ViewData.h"

class InstrumentView : public FieldView, public I_Observer {
public:
  InstrumentView(GUIWindow &w, ViewData *data);
  virtual ~InstrumentView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();
  virtual void AnimationUpdate();
  void onInstrumentTypeChange();
  void clearInstrumentModified() { instrumentModified_ = false; }

protected:
  void warpToNext(int offset);
  void onInstrumentChange();
  void fillMacroParameters();
  void fillSampleParameters();
  void fillSIDParameters();
  void fillMidiParameters();
  void fillOpalParameters();
  void fillNoneParameters();
  I_Instrument *getInstrument();
  void Update(Observable &o, I_ObservableData *d);
  void refreshInstrumentFields(const I_Instrument *old);

private:
  Project *project_;
  FourCC lastFocusID_;
  WatchedVariable instrumentType_;
  InstrumentType currentType_ = IT_NONE;
  bool instrumentModified_ = false;

  char sidName_[24];

  etl::vector<UIIntVarField, 1> typeIntVarField_;
  etl::vector<UIIntVarField, 40> intVarField_;
  etl::vector<UINoteVarField, 1> noteVarField_;
  etl::vector<UIStaticField, 4> staticField_;
  etl::vector<UIBigHexVarField, 4> bigHexVarField_;
  etl::vector<UIIntVarOffField, 1> intVarOffField_;
  etl::vector<UIBitmaskVarField, 3> bitmaskVarField_;
};
#endif
