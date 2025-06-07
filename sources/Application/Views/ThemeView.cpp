#include "ThemeView.h"
#include "Application/AppWindow.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/TextInputModalView.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include <Application/Model/ThemeConstants.h>

#define MAX_COLOR_VALUE 0xFFFFFF
#define FONT_FIELD_LINE 3

ThemeView::ThemeView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  GUIPoint position = GetAnchor();

  auto config = Config::GetInstance();

  Variable *v;

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
  position._y = FONT_FIELD_LINE;
  v = config->FindVariable(FourCC::VarUIFont);
  intVarField_.emplace_back(position, *v, "Font: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

  position._y += 2;

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
  position._y += 2;
  v = config->FindVariable(FourCC::VarFGColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Foreground: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_NORMAL, position);

  // Background color
  position._y += 1;
  v = config->FindVariable(FourCC::VarBGColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Background: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_BACKGROUND, position);

  // Highlight color
  position._y += 1;
  v = config->FindVariable(FourCC::VarHI1Color);
  bigHexVarField_.emplace_back(position, *v, 6, "Highlight1: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_HILITE1, position);

  // Highlight2 color
  position._y += 1;
  v = config->FindVariable(FourCC::VarHI2Color);
  bigHexVarField_.emplace_back(position, *v, 6, "Highlight2: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_HILITE2, position);

  // Console color
  position._y += 1;
  v = config->FindVariable(FourCC::VarConsoleColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Console:    %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_CONSOLE, position);

  // Cursor color
  position._y += 1;
  v = config->FindVariable(FourCC::VarCursorColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Cursor:     %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_CURSOR, position);

  // Info color
  position._y += 1;
  v = config->FindVariable(FourCC::VarInfoColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Info:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_INFO, position);

  // Warning color
  position._y += 1;
  v = config->FindVariable(FourCC::VarWarnColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Warning:    %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_WARN, position);

  // Error color
  position._y += 1;
  v = config->FindVariable(FourCC::VarErrorColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Error:      %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ERROR, position);

  // Play color
  position._y += 1;
  v = config->FindVariable(FourCC::VarAccentColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Accent:     %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ACCENT, position);

  // Mute color
  position._y += 1;
  v = config->FindVariable(FourCC::VarAccentAltColor);
  bigHexVarField_.emplace_back(position, *v, 6, "AccentAlt:  %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ACCENTALT, position);

  // Emphasis color
  position._y += 1;
  v = config->FindVariable(FourCC::VarEmphasisColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Emphasis:   %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_EMPHASIS, position);

  // dont show UI fields for reserved colors

  // // Reserved1 color
  // position._y += 1;
  // v = config->FindVariable(FourCC::VarAccentAltColor);
  // bigHexVarField_.emplace_back(position, *v, 6, "Reserved1: %6.6X", 0,
  //                              MAX_COLOR_VALUE, 16);
  // fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  // (*bigHexVarField_.rbegin()).AddObserver(*this);

  // addSwatchField(CD_RESERVED1, position);

  // // Reserved2 color
  // position._y += 1;
  // v = config->FindVariable(FourCC::VarReserved2Color);
  // bigHexVarField_.emplace_back(position, *v, 6, "Reserved2: %6.6X", 0,
  //                              MAX_COLOR_VALUE, 16);
  // fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  // (*bigHexVarField_.rbegin()).AddObserver(*this);

  // addSwatchField(CD_RESERVED2, position);

  // // Reserved3 color
  // position._y += 1;
  // v = config->FindVariable(FourCC::VarReserved3Color);
  // bigHexVarField_.emplace_back(position, *v, 6, "Reserved3: %6.6X", 0,
  //                              MAX_COLOR_VALUE, 16);
  // fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  // (*bigHexVarField_.rbegin()).AddObserver(*this);

  // addSwatchField(CD_RESERVED3, position);

  // // Reserved4 color
  // position._y += 1;
  // v = config->FindVariable(FourCC::VarReserved4Color);
  // bigHexVarField_.emplace_back(position, *v, 6, "Reserved4: %6.6X", 0,
  //                              MAX_COLOR_VALUE, 16);
  // fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  // (*bigHexVarField_.rbegin()).AddObserver(*this);

  // addSwatchField(CD_RESERVED4, position);
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

  SetColor(CD_NORMAL);
}

void ThemeView::addSwatchField(ColorDefinition color, GUIPoint position) {
  position._x -= 5;
  swatchField_.emplace_back(position, color);
  fieldList_.insert(fieldList_.end(), &(*swatchField_.rbegin()));
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
      // Save the config to persist the theme name
      config->Save();
    }
    return;
  }
  // if font changes call redraw all fields
  case FourCC::VarUIFont: {
    // need to force redraw of entire screen to update for font change
    ForceClear();
    DrawView();
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
      break;
    }
  default:
    NInvalid;
    break;
  };

  // Save config when any setting is changed
  Trace::Log("THEMEVIEW", "persist config changes!");
  Config *config = Config::GetInstance();
  config->Save();
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
      // Save the config to persist the theme name
      config->Save();
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
}