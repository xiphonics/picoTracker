#include "ThemeView.h"
#include "Application/AppWindow.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/TextInputModalView.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"

#define MAX_COLOR_VALUE 0xFFFFFF
#define FONT_FIELD_LINE 3

ThemeView::ThemeView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

  GUIPoint position = GetAnchor();

  auto config = Config::GetInstance();

  Variable *v;

  // Add import/export buttons at the top
  GUIPoint actionPos = position;
  actionPos._y += 1;

  actionField_.emplace_back("Import", FourCC::ActionImport, actionPos);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);

  actionPos._x += 8;
  actionField_.emplace_back("Export", FourCC::ActionExport, actionPos);
  fieldList_.insert(fieldList_.end(), &(*actionField_.rbegin()));
  (*actionField_.rbegin()).AddObserver(*this);

  // Font selection
  position._y = FONT_FIELD_LINE;
  v = config->FindVariable(FourCC::VarUIFont);
  intVarField_.emplace_back(position, *v, "Font: %s", 0, 1, 1, 1);
  fieldList_.insert(fieldList_.end(), &(*intVarField_.rbegin()));
  (*intVarField_.rbegin()).AddObserver(*this);

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
  v = config->FindVariable(FourCC::VarPlayColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Play:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_PLAY, position);

  // Mute color
  position._y += 1;
  v = config->FindVariable(FourCC::VarMuteColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Mute:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_MUTE, position);

  // SongViewFE color
  position._y += 1;
  v = config->FindVariable(FourCC::VarSongViewFEColor);
  bigHexVarField_.emplace_back(position, *v, 6, "SongViewFE: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_SONGVIEWFE, position);

  // SongView00 color
  position._y += 1;
  v = config->FindVariable(FourCC::VarSongView00Color);
  bigHexVarField_.emplace_back(position, *v, 6, "SongView00: %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_SONGVIEW00, position);

  // Row color
  position._y += 1;
  v = config->FindVariable(FourCC::VarRowColor);
  bigHexVarField_.emplace_back(position, *v, 6, "Row:        %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ROW, position);

  // Row2 color
  position._y += 1;
  v = config->FindVariable(FourCC::VarRow2Color);
  bigHexVarField_.emplace_back(position, *v, 6, "Row2:       %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_ROW2, position);

  // MajorBeat color
  position._y += 1;
  v = config->FindVariable(FourCC::VarMajorBeatColor);
  bigHexVarField_.emplace_back(position, *v, 6, "MajorBeat:  %6.6X", 0,
                               MAX_COLOR_VALUE, 16);
  fieldList_.insert(fieldList_.end(), &(*bigHexVarField_.rbegin()));
  (*bigHexVarField_.rbegin()).AddObserver(*this);

  addSwatchField(CD_MAJORBEAT, position);
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
    exportTheme();
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
  case FourCC::VarPlayColor:
  case FourCC::VarMuteColor:
  case FourCC::VarSongViewFEColor:
  case FourCC::VarSongView00Color:
  case FourCC::VarRowColor:
  case FourCC::VarRow2Color:
  case FourCC::VarMajorBeatColor: {
    // Update the AppWindow's color values from Config
    ((AppWindow &)w_).UpdateColorsFromConfig();

    // Force a redraw of the entire screen to update all colors
    ForceClear();
    DrawView();
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

// No need for forward declarations with lambdas

// Forward declarations for static callback functions
static void ExportThemeInputCallback(View &v, ModalView &dialog);
static void OverwriteThemeCallback(View &v, ModalView &dialog);

void ThemeView::exportTheme() {
  // Create a text input dialog for the theme name
  TextInputModalView *tiv = new TextInputModalView(
      *this, "Export Theme", "Name:", MBBF_OK | MBBF_CANCEL, exportThemeName_);

  DoModal(tiv, ExportThemeInputCallback);
}

// Callback for the theme name input dialog
static void ExportThemeInputCallback(View &v, ModalView &dialog) {
  ThemeView &tv = (ThemeView &)v;
  auto retCode = dialog.GetReturnCode();
  if (retCode == MBL_OK) {
    // Get the text from the text field
    TextInputModalView &inputView = (TextInputModalView &)dialog;
    etl::string<MAX_TEXT_INPUT_LENGTH> themeName =
        inputView.GetTextField()->GetString();

    // Don't proceed if the name is empty
    if (themeName.empty()) {
      return;
    }

    tv.exportThemeName_ = themeName;

    auto fs = FileSystem::GetInstance();

    // Check if theme with this name already exists
    etl::string<MAX_INSTRUMENT_NAME_LENGTH + 4> filename = themeName;
    filename.append(THEME_FILE_EXTENSION);

    etl::string<MAX_INSTRUMENT_NAME_LENGTH + 10> path = THEMES_DIR;
    path.append("/");
    path.append(filename);

    if (fs->exists(path.c_str())) {
      // Theme already exists, ask for confirmation to overwrite
      MessageBox *mb = new MessageBox(tv, "Theme already exists. Overwrite?",
                                      MBBF_YES | MBBF_NO);

      tv.DoModal(mb, OverwriteThemeCallback);
    } else {
      // Theme doesn't exist, export it directly
      Theme theme;
      theme.SaveFromConfig(); // Save current config settings to the theme

      // Export with overwrite flag set to false
      PersistencyResult result =
          PersistencyService::GetInstance()->ExportTheme(
              &theme, tv.exportThemeName_, false);

      if (result == PERSIST_SAVED) {
        MessageBox *successMb =
            new MessageBox(tv, "Theme exported successfully", MBBF_OK);
        tv.DoModal(successMb);
      } else {
        MessageBox *errorMb =
            new MessageBox(tv, "Failed to export theme", MBBF_OK);
        tv.DoModal(errorMb);
      }
    }
  }
}

// Callback for the overwrite confirmation dialog
static void OverwriteThemeCallback(View &v, ModalView &dialog) {
  ThemeView &tv = (ThemeView &)v;
  if (dialog.GetReturnCode() == MBL_YES) {
    // User confirmed overwrite, export the theme
    Theme theme;
    theme.SaveFromConfig(); // Save current config settings to the theme

    // Export with overwrite flag set to true
    PersistencyResult result =
        PersistencyService::GetInstance()->ExportTheme(
            &theme, tv.exportThemeName_, true);

    if (result == PERSIST_SAVED) {
      MessageBox *successMb =
          new MessageBox(tv, "Theme exported successfully", MBBF_OK);
      tv.DoModal(successMb);
    } else {
      MessageBox *errorMb =
          new MessageBox(tv, "Failed to export theme", MBBF_OK);
      tv.DoModal(errorMb);
    }
  }
}

void ThemeView::importTheme() {
  // Switch to the theme import view
  ViewType vt = VT_THEME_IMPORT;
  ViewEvent ve(VET_SWITCH_VIEW, &vt);
  SetChanged();
  NotifyObservers(&ve);
}
