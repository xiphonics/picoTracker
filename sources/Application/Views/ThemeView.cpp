/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ThemeView.h"
#include "Application/AppWindow.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/TextInputModalView.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include <Application/Model/ThemeConstants.h>
#include <stdint.h>

#define FONT_FIELD_LINE 3

#define COLOR_LABEL_WIDTH 12
#define COMPONENT_SPACING 3

constexpr uint8_t COLOR_COMPONENT_X_COL_POS[COLOR_COMPONENT_COUNT] = {16, 8, 0};
constexpr uint8_t COLOR_COMPONENT_X_OFFSETS[COLOR_COMPONENT_COUNT] = {
    COLOR_LABEL_WIDTH, COLOR_LABEL_WIDTH + COMPONENT_SPACING,
    COLOR_LABEL_WIDTH + 2 * COMPONENT_SPACING};

ThemeView::ThemeView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  GUIPoint position = GetAnchor();

  auto config = Config::GetInstance();

  // Add import/export buttons at the top
  GUIPoint actionPos = position;

  actionPos._y -= 1;

  actionField_.emplace_back("Import", FourCC::ActionImport, actionPos);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);

  actionPos._x += 8;
  actionField_.emplace_back("Export", FourCC::ActionExport, actionPos);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);
  actionPos._y += 1;

  // Font selection
  // BUT Not on the Advance where there is currently only a single font
#ifndef ADV
  position._y = FONT_FIELD_LINE;
  Variable *fontVar = config->FindVariable(FourCC::VarUIFont);
  intVarField_.emplace_back(position, *fontVar, "Font: %s", 0,
                            ThemeConstants::FONT_COUNT - 1, 1,
                            ThemeConstants::FONT_COUNT - 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);
  position._y += 2;
#else
  position._y += 1;
#endif

  // Get the current theme name from Config
  Variable *configThemeVar = config->FindVariable(FourCC::VarThemeName);
  etl::string<MAX_THEME_NAME_LENGTH> currentThemeName = "default";

  // If the theme name is set in the config, use it
  if (configThemeVar && !configThemeVar->GetString().empty()) {
    currentThemeName = configThemeVar->GetString();
  }

  // Create a variable for the theme name field
  Variable *themeNameVar =
      new Variable(FourCC::ActionThemeName, currentThemeName.c_str());

  // Create the label and default value as variables to avoid temporary objects
  auto label = etl::string<MAX_UITEXTFIELD_LABEL_LENGTH>("Theme: ");
  auto defaultValue = etl::string<MAX_THEME_NAME_LENGTH>(currentThemeName);

  // Add the text field
  textFields_.emplace_back(*themeNameVar, position, label,
                           FourCC::ActionThemeName, defaultValue);
  themeNameField_ = &(*textFields_.rbegin());
  themeNameField_->AddObserver(*this);
  fieldList_.insert(fieldList_.end(), themeNameField_);

  // Initialize the edit mode flag
  themeNameEditMode_ = false;

  // Initialize the export theme name
  exportThemeName_ = currentThemeName;

  // Foreground color
  position._y += 3;
  addColorField("Foreground", config->FindVariable(FourCC::VarFGColor),
                CD_NORMAL, position);

  // Background color
  position._y += 1;
  addColorField("Background", config->FindVariable(FourCC::VarBGColor),
                CD_BACKGROUND, position);

  // Highlight color
  position._y += 1;
  addColorField("Highlight1", config->FindVariable(FourCC::VarHI1Color),
                CD_HILITE1, position);

  // Highlight2 color
  position._y += 1;
  addColorField("Highlight2", config->FindVariable(FourCC::VarHI2Color),
                CD_HILITE2, position);

  // Console color
  position._y += 1;
  addColorField("Console", config->FindVariable(FourCC::VarConsoleColor),
                CD_CONSOLE, position);

  // Cursor color
  position._y += 1;
  addColorField("Cursor", config->FindVariable(FourCC::VarCursorColor),
                CD_CURSOR, position);

  // Info color
  position._y += 1;
  addColorField("Info", config->FindVariable(FourCC::VarInfoColor), CD_INFO,
                position);

  // Warning color
  position._y += 1;
  addColorField("Warning", config->FindVariable(FourCC::VarWarnColor), CD_WARN,
                position);

  // Error color
  position._y += 1;
  addColorField("Error", config->FindVariable(FourCC::VarErrorColor), CD_ERROR,
                position);

  // Play color
  position._y += 1;
  addColorField("Accent", config->FindVariable(FourCC::VarAccentColor),
                CD_ACCENT, position);

  // Mute color
  position._y += 1;
  addColorField("AccentAlt", config->FindVariable(FourCC::VarAccentAltColor),
                CD_ACCENTALT, position);

  // Emphasis color
  position._y += 1;
  addColorField("Emphasis", config->FindVariable(FourCC::VarEmphasisColor),
                CD_EMPHASIS, position);
}

ThemeView::~ThemeView() {}

void ThemeView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed)
    return;

  FieldView::ProcessButtonMask(mask, pressed);

  if (mask & EPBM_NAV) {
    if (mask & EPBM_LEFT) {
      // Go back to Device view with NAV+LEFT
      ViewType vt = VT_DEVICE;
      ViewEvent ve(VET_SWITCH_VIEW, &vt);
      SetChanged();
      NotifyObservers(&ve);
    }
  } else if (mask & EPBM_PLAY) {
    Player *player = Player::GetInstance();
    player->OnStartButton(PM_SONG, viewData_->songX_, false, viewData_->songX_);
  }
}

void ThemeView::DrawView() {
  Clear();

  GUITextProperties props;
  GUIPoint pos = GetTitlePosition();

  // Draw title
  char titleString[SCREEN_WIDTH];
  strcpy(titleString, "Theme Settings");

  SetColor(CD_NORMAL);
  DrawString(pos._x, pos._y, titleString, props);

  // bit of a hack needed for font change as going from "standard" to "bold"
  // will leave behind partial characters due to different width of those string
  // labels
  DrawString(5, FONT_FIELD_LINE, "                            ", props);

  FieldView::Redraw();

  // just draw the RGB column headings directly:
  SetColor(CD_INFO);
  GUITextProperties headerProps;
  DrawString(17, 6, "R  G  B", headerProps);
  SetColor(CD_NORMAL);
}

void ThemeView::addSwatchField(ColorDefinition color, GUIPoint position) {
  position._x -= 5;
  swatchField_.emplace_back(position, color);
  fieldList_.insert(fieldList_.end(), &(*swatchField_.rbegin()));
}

void ThemeView::addColorField(const char *label, Variable *colorVar,
                              ColorDefinition color, GUIPoint position) {

  staticField_.emplace_back(position, label);
  UIStaticField &labelField = *staticField_.rbegin();
  fieldList_.insert(fieldList_.end(), &labelField);

  uint32_t colorValue = static_cast<uint32_t>(colorVar->GetInt());

  for (uint8_t i = 0; i < COLOR_COMPONENT_COUNT; ++i) {
    GUIPoint componentPosition = position;
    componentPosition._x += COLOR_COMPONENT_X_OFFSETS[i];

    colorComponentVars_.emplace_back(
        colorVar->GetID(),
        static_cast<int>((colorValue >> COLOR_COMPONENT_X_COL_POS[i]) &
                         static_cast<uint32_t>(0xFF)));
    Variable &componentVar = *colorComponentVars_.rbegin();

    // bigger steps and set limits because we use RGB565 for colors
    if (i == 0 || i == 2) {
      intVarField_.emplace_back(componentPosition, componentVar, "%2.2X", 0,
                                248, 8, 16, 0);
    } else {
      intVarField_.emplace_back(componentPosition, componentVar, "%2.2X", 0,
                                252, 4, 16, 0);
    }
    UIIntVarField &componentField = *intVarField_.rbegin();
    fieldList_.insert(fieldList_.end(), &componentField);
    componentField.AddObserver(*this);

    colorComponentFields_.emplace_back();
    auto &fieldInfo = colorComponentFields_.back();
    fieldInfo.observable = &componentField;
    fieldInfo.componentVar = &componentVar;
    fieldInfo.colorVar = colorVar;
    fieldInfo.shift = COLOR_COMPONENT_X_COL_POS[i];
  }

  addSwatchField(color, position);
}

ThemeView::ColorComponentField *
ThemeView::findColorComponentField(Observable *observable) {
  for (auto &entry : colorComponentFields_) {
    if (entry.observable == observable) {
      return &entry;
    }
  }
  return nullptr;
}

void ThemeView::syncColorComponentVars(Variable *colorVar) {
  if (colorVar == nullptr) {
    return;
  }
  uint32_t colorValue = static_cast<uint32_t>(colorVar->GetInt());
  for (auto &entry : colorComponentFields_) {
    if (entry.colorVar != colorVar) {
      continue;
    }
    uint32_t componentValue =
        (colorValue >> entry.shift) & static_cast<uint32_t>(0xFF);
    entry.componentVar->SetInt(static_cast<int>(componentValue), false);
  }
}

void ThemeView::Update(Observable &o, I_ObservableData *d) {
  if (!hasFocus_) {
    return;
  }
  UIField *focus = GetFocus();
  focus->ClearFocus();
  focus->Draw(w_);
  w_.Flush();
  focus->SetFocus();
  focus->Draw(w_);
  isDirty_ = true;

  uintptr_t fourcc = (uintptr_t)d;

  ColorComponentField *componentField = findColorComponentField(&o);
  if (componentField != nullptr) {
    Variable *colorVar = componentField->colorVar;
    uint32_t colorValue = colorVar->GetInt();
    uint32_t newComponentValue = componentField->componentVar->GetInt() & 0xFF;
    colorValue &= ~(static_cast<uint32_t>(0xFF) << componentField->shift);
    colorValue |= newComponentValue << componentField->shift;
    colorVar->SetInt(static_cast<int>(colorValue));
    syncColorComponentVars(colorVar);
    fourcc = colorVar->GetID();
  }

  switch (fourcc) {
  // Handle theme import action
  case FourCC::ActionImport: {
    // Switch to the ThemeImportView
    ViewType vt = VT_THEME_IMPORT;
    ViewEvent ve(VET_SWITCH_VIEW, &vt);
    SetChanged();
    NotifyObservers(&ve);
    return;
  }
  // Handle theme export action
  case FourCC::ActionExport: {
    // Get the theme name from the text field
    exportThemeName_ = themeNameField_->GetString();

    // Check if the theme name is empty
    if (exportThemeName_.empty()) {
      exportThemeName_ = ThemeConstants::DEFAULT_THEME_NAME;
      themeNameField_->SetVariable(
          *new Variable(FourCC::ActionThemeName, exportThemeName_.c_str()));
    }

    // Export the theme
    handleThemeExport();
    return;
  }
  // Handle theme name field
  case FourCC::ActionThemeName: {
    // Update the export theme name
    exportThemeName_ = themeNameField_->GetString();

    // Update the theme name in the Config
    Config *config = Config::GetInstance();
    Variable *themeNameVar = config->FindVariable(FourCC::VarThemeName);
    if (themeNameVar) {
      themeNameVar->SetString(exportThemeName_.c_str());
      configDirty_ = true;
    }
    return;
  }
  // if font changes call redraw all fields
  case FourCC::VarUIFont: {
    // need to force redraw of entire screen to update for font change
    ForceClear();
    DrawView();
    configDirty_ = true;
    break;
  }
  // Handle color variable changes
  case FourCC::VarBGColor:
  case FourCC::VarFGColor:
  case FourCC::VarHI1Color:
  case FourCC::VarHI2Color:
  case FourCC::VarConsoleColor:
  case FourCC::VarCursorColor:
  case FourCC::VarInfoColor:
  case FourCC::VarWarnColor:
  case FourCC::VarErrorColor:
  case FourCC::VarAccentColor:
  case FourCC::VarAccentAltColor:
  case FourCC::VarEmphasisColor:
    // case FourCC::VarReserved1Color:
    // case FourCC::VarReserved2Color:
    // case FourCC::VarReserved3Color:
    // case FourCC::VarReserved4Color:
    {
      // Update the AppWindow's color values from Config
      ((AppWindow &)w_).UpdateColorsFromConfig();

      // Force a redraw of the entire screen to update all colors
      _forceRedraw = true;
      configDirty_ = true;
      break;
    }
  default:
    NInvalid;
    break;
  };
}

void ThemeView::handleThemeExport() {
  // Check if the theme name is valid
  if (exportThemeName_.empty()) {
    exportThemeName_ = ThemeConstants::DEFAULT_THEME_NAME;
  }

  // Build the path to check if the theme already exists
  char pathBuffer[MAX_THEME_EXPORT_PATH_LENGTH + 1];
  memset(pathBuffer, 0, sizeof(pathBuffer));

  strcpy(pathBuffer, THEMES_DIR);
  strcat(pathBuffer, "/");
  strcat(pathBuffer, exportThemeName_.c_str());
  strcat(pathBuffer, THEME_FILE_EXTENSION);

  // Check if theme exists
  auto fs = FileSystem::GetInstance();
  if (fs->exists(pathBuffer)) {
    // Theme exists, ask for confirmation
    MessageBox *mb = new MessageBox(*this, "Theme already exists. Overwrite?",
                                    MBBF_YES | MBBF_NO);

    // Use a lambda for the callback to avoid the static function
    DoModal(mb, [this](View &v, ModalView &dialog) {
      if (dialog.GetReturnCode() == MBL_YES) {
        // User confirmed overwrite
        exportThemeWithName(exportThemeName_.c_str(), true);
      }
    });
  } else {
    // Theme doesn't exist, export directly
    exportThemeWithName(exportThemeName_.c_str(), false);
  }
}

void ThemeView::exportThemeWithName(const char *themeName, bool overwrite) {
  // Export the theme using Config
  Config *config = Config::GetInstance();
  bool result = config->ExportTheme(themeName, overwrite);

  if (result) {
    // Update the theme name in the Config
    Variable *themeNameVar = config->FindVariable(FourCC::VarThemeName);
    if (themeNameVar) {
      themeNameVar->SetString(themeName);
      configDirty_ = true;
    }

    // Update the theme name field
    themeNameField_->SetVariable(
        *new Variable(FourCC::ActionThemeName, themeName));
    exportThemeName_ = themeName;
  }

  // Show result message
  MessageBox *resultMb = new MessageBox(
      *this, result ? "Theme exported successfully" : "Failed to export theme",
      MBBF_OK);
  DoModal(resultMb);
}

void ThemeView::updateThemeNameFromConfig() {
  // Get the current theme name from Config
  Config *config = Config::GetInstance();
  Variable *themeNameVar = config->FindVariable(FourCC::VarThemeName);

  if (themeNameVar && !themeNameVar->GetString().empty()) {
    // Get the theme name from Config
    etl::string<MAX_THEME_NAME_LENGTH> themeName = themeNameVar->GetString();

    // Update the theme name field
    themeNameField_->SetVariable(
        *new Variable(FourCC::ActionThemeName, themeName.c_str()));
    exportThemeName_ = themeName;
  }
}

void ThemeView::OnFocus() {
  // Update the theme name field from Config when the view gets focus
  // This ensures the field is updated after importing a theme
  updateThemeNameFromConfig();
}

void ThemeView::OnFocusLost() {
  if (!configDirty_) {
    return;
  }

  Config *config = Config::GetInstance();
  if (!config->Save()) {
    Trace::Error("THEMEVIEW", "Failed to save theme config on focus lost");
    return;
  }

  Trace::Log("THEMEVIEW", "Saved theme config on focus lost");
  configDirty_ = false;
}

// Keep this method for backward compatibility
void ThemeView::exportTheme() {
  // This now just calls handleThemeExport
  handleThemeExport();
}

// We've replaced the static callbacks with lambdas and direct methods

void ThemeView::importTheme() {
  // Switch to the theme import view
  ViewType vt = VT_THEME_IMPORT;
  ViewEvent ve(VET_SWITCH_VIEW, &vt);
  SetChanged();
  NotifyObservers(&ve);
}
void ThemeView::AnimationUpdate() {
  if (_forceRedraw) {
    ForceClear();
    DrawView();
    _forceRedraw = false;
  }
  GUITextProperties props;
  drawBattery(props);
  drawPowerButtonUI(props);
}
