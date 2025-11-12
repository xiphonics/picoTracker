/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _THEME_VIEW_H_
#define _THEME_VIEW_H_

#include "Application/Model/Config.h"
#include "Application/Views/BaseClasses/ModalView.h"
#include "Application/Views/UIController.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "BaseClasses/UISwatchField.h"
#include "BaseClasses/UITextField.h"
#include "BaseClasses/View.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/T_SimpleList.h"
#include "Foundation/T_Stack.h"
#include "ViewData.h"


#define COLOR_COMPONENT_COUNT 3
#define COLOR_COUNT 12
#define COLOR_SUB_FIELDS_COUNT (COLOR_COMPONENT_COUNT * COLOR_COUNT)

class ThemeView : public FieldView, public I_Observer {
public:
  ThemeView(GUIWindow &w, ViewData *data);
  virtual ~ThemeView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  void OnFocus() override;
  void OnFocusLost() override;

  // Observer for action callback
  void Update(Observable &, I_ObservableData *);

  void AnimationUpdate() override;

  // For storing export theme name during modal callbacks
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> exportThemeName_;

protected:
private:
  void addSwatchField(ColorDefinition color, GUIPoint position);
  void addColorField(const char *label, Variable *colorVar,
                      ColorDefinition color, GUIPoint position);
  void syncColorComponentVars(Variable *colorVar);

  struct ColorComponentField {
    Observable *observable = nullptr;
    Variable *componentVar = nullptr;
    Variable *colorVar = nullptr;
    uint8_t shift = 0;
  };

  ColorComponentField *findColorComponentField(Observable *observable);

  etl::vector<UIIntVarField, (COLOR_SUB_FIELDS_COUNT + 1)>
      intVarField_; // for colors + 1 for font selector
  etl::vector<UISwatchField, COLOR_COUNT> swatchField_;
  etl::vector<UIStaticField, COLOR_COUNT> staticField_;
  etl::vector<Variable, COLOR_SUB_FIELDS_COUNT> colorComponentVars_;
  etl::vector<ColorComponentField, COLOR_SUB_FIELDS_COUNT> colorComponentFields_;
  etl::vector<UIActionField, 2> actionField_; // For Import/Export buttons
  etl::vector<UITextField<MAX_THEME_NAME_LENGTH>, 1>
      textFields_; // For theme name input

  // Reference to the theme name field for direct access
  UITextField<MAX_THEME_NAME_LENGTH> *themeNameField_;
  bool themeNameEditMode_; // Flag to track if we're editing the theme name

  // Helper methods for theme import/export
  void handleThemeExport();
  void exportTheme();
  void importTheme();
  void exportThemeWithName(const char *themeName, bool overwrite);
  void updateThemeNameFromConfig(); // Update the theme name field from Config

  // need separate flag for force because isDirty_ won't work for this because
  // it gets reset before AnimationUpdate is called in ThemeViews case
  bool _forceRedraw = false;
  // use to flag pending theme data changes to be persisted to file
  bool configDirty_ = false;
};
#endif
