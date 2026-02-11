/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

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
#include "Externals/etl/include/etl/string.h"
#include "Externals/etl/include/etl/vector.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/Variable.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "ViewData.h"
#include <cstddef>

class SampleInstrument;

class InstrumentView : public FieldView, public I_Observer {
public:
  InstrumentView(GUIWindow &w, ViewData *data);
  virtual ~InstrumentView();
  void Reset();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();
  void onInstrumentTypeChange(bool updateUI = false);
  bool checkInstrumentModified();
  void resetInstrumentToDefaults();

  // only public to allow to be called from modal dialog static callback
  void applyProposedTypeChangeUI();

protected:
  void warpToNext(int offset);
  void onInstrumentChange();
  void fillSampleParameters();
  void fillSIDParameters();
  void fillMidiParameters();
  void fillOpalParameters();
  void fillNoneParameters();
  I_Instrument *getInstrument();
  void Update(Observable &o, I_ObservableData *d);
  void refreshInstrumentFields();
  void addNameTextField(I_Instrument *instr, GUIPoint &position);
  void handleInstrumentExport();

private:
  static constexpr size_t SliceCountLabelSize = 20;
  Project *project_;
  FourCC lastFocusID_;
  WatchedVariable instrumentType_;
  int lastSampleIndex_;
  bool suppressSampleChangeWarning_;
  etl::string<SliceCountLabelSize> sliceCountLabel_;

  // Variables for export confirmation dialog
  I_Instrument *exportInstrument_ = nullptr;
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> exportName_;
  InstrumentType pendingInstrumentType_ = IT_NONE;

  etl::vector<UIIntVarField, 1> typeIntVarField_;
  etl::vector<UIActionField, 2> persistentActionField_;
  etl::vector<UIIntVarField, 40> intVarField_;
  etl::vector<UINoteVarField, 1> noteVarField_;
  etl::vector<UIStaticField, 10> staticField_;
  etl::vector<UIBigHexVarField, 4> bigHexVarField_;
  etl::vector<UIIntVarOffField, 2> intVarOffField_;
  etl::vector<UIActionField, 1> sampleActionField_;
  etl::vector<UIBitmaskVarField, 3> bitmaskVarField_;
  etl::vector<UITextField<MAX_INSTRUMENT_NAME_LENGTH>, 1> nameTextField_;
  etl::vector<InstrumentNameVariable, 1> nameVariables_;
};
#endif
