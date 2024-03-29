#ifndef _INSTRUMENT_VIEW_H_
#define _INSTRUMENT_VIEW_H_

#include "BaseClasses/FieldView.h"
#include "BaseClasses/UIBigHexVarField.h"
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
  virtual void AnimationUpdate(){};

protected:
  void warpToNext(int offset);
  void onInstrumentChange();
  void fillSampleParameters();
  void fillMidiParameters();
  void fillTinysynthParameters();
  InstrumentType getInstrumentType();
  void Update(Observable &o, I_ObservableData *d);

private:
  Project *project_;
  FourCC lastFocusID_;
  I_Instrument *current_;

  etl::vector<UIIntVarField, 15> intVarField_;
  etl::vector<UINoteVarField, 1> noteVarField_;
  etl::vector<UIStaticField, 1> staticField_;
  etl::vector<UIBigHexVarField, 3> bigHexVarField_;
  etl::vector<UIIntVarOffField, 1> intVarOffField_;
};
#endif
