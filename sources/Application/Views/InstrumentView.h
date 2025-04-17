#ifndef _INSTRUMENT_VIEW_H_
#define _INSTRUMENT_VIEW_H_

#include "Application/Instruments/InstrumentNameVariable.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIBitmaskVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIIntVarOffField.h"
#include "BaseClasses/UINoteVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "BaseClasses/UITextField.h"
#include "Externals/etl/include/etl/vector.h"
#include "FieldView.h"
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
  void addNameTextField(I_Instrument *instr, GUIPoint &position);
  void handleInstrumentExport();
  void setFocus(FourCC id);

private:
  Project *project_;
  FourCC lastFocusID_;
  WatchedVariable instrumentType_;
  InstrumentType currentType_ = IT_NONE;
  bool instrumentModified_ = false;

  // Variables for export confirmation dialog
  I_Instrument *exportInstrument_ = nullptr;
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> exportName_;

  etl::vector<UIIntVarField, 1> typeIntVarField_;
  etl::vector<UIActionField, 2> actionField_;
  etl::vector<UIIntVarField, 40> intVarField_;
  etl::vector<UINoteVarField, 1> noteVarField_;
  etl::vector<UIStaticField, 4> staticField_;
  etl::vector<UIBigHexVarField, 4> bigHexVarField_;
  etl::vector<UIIntVarOffField, 1> intVarOffField_;
  etl::vector<UIBitmaskVarField, 3> bitmaskVarField_;
  etl::vector<UITextField<MAX_INSTRUMENT_NAME_LENGTH>, 1> nameTextField_;
  etl::vector<InstrumentNameVariable, 1> nameVariables_;
};
#endif
