#include "Theme.h"
#include "Application/AppWindow.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include "ThemeConstants.h"
#include <stdlib.h> // For atoi
#include <string.h> // For strcmp

Theme::Theme() : Persistent("THEME") {
  // Initialize with default values
  name_ = "Default";
  fontValue_ = ThemeConstants::DEFAULT_UIFONT;
  bgColor_ = ThemeConstants::DEFAULT_BACKGROUND;
  fgColor_ = ThemeConstants::DEFAULT_FOREGROUND;
  hi1Color_ = ThemeConstants::DEFAULT_HICOLOR1;
  hi2Color_ = ThemeConstants::DEFAULT_HICOLOR2;
  consoleColor_ = ThemeConstants::DEFAULT_CONSOLECOLOR;
  cursorColor_ = ThemeConstants::DEFAULT_CURSORCOLOR;
  infoColor_ = ThemeConstants::DEFAULT_INFOCOLOR;
  warnColor_ = ThemeConstants::DEFAULT_WARNCOLOR;
  errorColor_ = ThemeConstants::DEFAULT_ERRORCOLOR;
  playColor_ = ThemeConstants::DEFAULT_PLAYCOLOR;
  muteColor_ = ThemeConstants::DEFAULT_MUTECOLOR;
  songViewFEColor_ = ThemeConstants::DEFAULT_SONGVIEWFECOLOR;
  songView00Color_ = ThemeConstants::DEFAULT_SONGVIEW00COLOR;
  rowColor_ = ThemeConstants::DEFAULT_ROWCOLOR;
  row2Color_ = ThemeConstants::DEFAULT_ROW2COLOR;
  majorBeatColor_ = ThemeConstants::DEFAULT_MAJORBEATCOLOR;
}

Theme::~Theme() {}

void Theme::SetName(const char *name) { name_ = name; }

const char *Theme::GetName() const { return name_.c_str(); }

void Theme::SaveFromConfig() {
  Config *config = Config::GetInstance();
  Trace::Log("THEME", "Saving theme settings from config");

  // Save font setting
  Variable *fontVar = config->FindVariable(FourCC::VarUIFont);
  if (fontVar) {
    fontValue_ = fontVar->GetInt();
  }

  // Save color settings
  Variable *bgVar = config->FindVariable(FourCC::VarBGColor);
  if (bgVar) {
    bgColor_ = bgVar->GetInt();
  }

  Variable *fgVar = config->FindVariable(FourCC::VarFGColor);
  if (fgVar) {
    fgColor_ = fgVar->GetInt();
  }

  Variable *hi1Var = config->FindVariable(FourCC::VarHI1Color);
  if (hi1Var) {
    hi1Color_ = hi1Var->GetInt();
  }

  Variable *hi2Var = config->FindVariable(FourCC::VarHI2Color);
  if (hi2Var) {
    hi2Color_ = hi2Var->GetInt();
  }

  Variable *consoleVar = config->FindVariable(FourCC::VarConsoleColor);
  if (consoleVar) {
    consoleColor_ = consoleVar->GetInt();
  }

  Variable *cursorVar = config->FindVariable(FourCC::VarCursorColor);
  if (cursorVar) {
    cursorColor_ = cursorVar->GetInt();
  }

  Variable *infoVar = config->FindVariable(FourCC::VarInfoColor);
  if (infoVar) {
    infoColor_ = infoVar->GetInt();
  }

  Variable *warnVar = config->FindVariable(FourCC::VarWarnColor);
  if (warnVar) {
    warnColor_ = warnVar->GetInt();
  }

  Variable *errorVar = config->FindVariable(FourCC::VarErrorColor);
  if (errorVar) {
    errorColor_ = errorVar->GetInt();
  }

  Variable *playVar = config->FindVariable(FourCC::VarPlayColor);
  if (playVar) {
    playColor_ = playVar->GetInt();
  }

  Variable *muteVar = config->FindVariable(FourCC::VarMuteColor);
  if (muteVar) {
    muteColor_ = muteVar->GetInt();
  }

  Variable *songViewFEVar = config->FindVariable(FourCC::VarSongViewFEColor);
  if (songViewFEVar) {
    songViewFEColor_ = songViewFEVar->GetInt();
  }

  Variable *songView00Var = config->FindVariable(FourCC::VarSongView00Color);
  if (songView00Var) {
    songView00Color_ = songView00Var->GetInt();
  }

  Variable *rowVar = config->FindVariable(FourCC::VarRowColor);
  if (rowVar) {
    rowColor_ = rowVar->GetInt();
  }

  Variable *row2Var = config->FindVariable(FourCC::VarRow2Color);
  if (row2Var) {
    row2Color_ = row2Var->GetInt();
  }

  Variable *majorBeatVar = config->FindVariable(FourCC::VarMajorBeatColor);
  if (majorBeatVar) {
    majorBeatColor_ = majorBeatVar->GetInt();
  }
}

void Theme::ApplyToConfig() {
  Config *config = Config::GetInstance();
  Trace::Log("THEME", "Applying theme settings to config");

  // Apply font setting
  Variable *fontVar = config->FindVariable(FourCC::VarUIFont);
  if (fontVar) {
    fontVar->SetInt(fontValue_);
  }

  // Apply color settings
  Variable *bgVar = config->FindVariable(FourCC::VarBGColor);
  if (bgVar) {
    bgVar->SetInt(bgColor_);
  }

  Variable *fgVar = config->FindVariable(FourCC::VarFGColor);
  if (fgVar) {
    fgVar->SetInt(fgColor_);
  }

  Variable *hi1Var = config->FindVariable(FourCC::VarHI1Color);
  if (hi1Var) {
    hi1Var->SetInt(hi1Color_);
  }

  Variable *hi2Var = config->FindVariable(FourCC::VarHI2Color);
  if (hi2Var) {
    hi2Var->SetInt(hi2Color_);
  }

  Variable *consoleVar = config->FindVariable(FourCC::VarConsoleColor);
  if (consoleVar) {
    consoleVar->SetInt(consoleColor_);
  }

  Variable *cursorVar = config->FindVariable(FourCC::VarCursorColor);
  if (cursorVar) {
    cursorVar->SetInt(cursorColor_);
  }

  Variable *infoVar = config->FindVariable(FourCC::VarInfoColor);
  if (infoVar) {
    infoVar->SetInt(infoColor_);
  }

  Variable *warnVar = config->FindVariable(FourCC::VarWarnColor);
  if (warnVar) {
    warnVar->SetInt(warnColor_);
  }

  Variable *errorVar = config->FindVariable(FourCC::VarErrorColor);
  if (errorVar) {
    errorVar->SetInt(errorColor_);
  }

  Variable *playVar = config->FindVariable(FourCC::VarPlayColor);
  if (playVar) {
    playVar->SetInt(playColor_);
  }

  Variable *muteVar = config->FindVariable(FourCC::VarMuteColor);
  if (muteVar) {
    muteVar->SetInt(muteColor_);
  }

  Variable *songViewFEVar = config->FindVariable(FourCC::VarSongViewFEColor);
  if (songViewFEVar) {
    songViewFEVar->SetInt(songViewFEColor_);
  }

  Variable *songView00Var = config->FindVariable(FourCC::VarSongView00Color);
  if (songView00Var) {
    songView00Var->SetInt(songView00Color_);
  }

  Variable *rowVar = config->FindVariable(FourCC::VarRowColor);
  if (rowVar) {
    rowVar->SetInt(rowColor_);
  }

  Variable *row2Var = config->FindVariable(FourCC::VarRow2Color);
  if (row2Var) {
    row2Var->SetInt(row2Color_);
  }

  Variable *majorBeatVar = config->FindVariable(FourCC::VarMajorBeatColor);
  if (majorBeatVar) {
    majorBeatVar->SetInt(majorBeatColor_);
  }
}

void Theme::SaveContent(tinyxml2::XMLPrinter *printer) {
  Trace::Log("THEME", "Saving theme content to XML");

  // Save theme name
  printer->OpenElement("Name");
  printer->PushText(name_.c_str());
  printer->CloseElement(); // Name

  // Save the font setting
  printer->OpenElement("Font");
  printer->PushAttribute("value", fontValue_);
  printer->CloseElement(); // Font

  // Save all color settings
  printer->OpenElement("Colors");

  // Background color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Background");
  printer->PushAttribute("value", bgColor_);
  printer->CloseElement();

  // Foreground color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Foreground");
  printer->PushAttribute("value", fgColor_);
  printer->CloseElement();

  // Highlight 1 color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Highlight1");
  printer->PushAttribute("value", hi1Color_);
  printer->CloseElement();

  // Highlight 2 color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Highlight2");
  printer->PushAttribute("value", hi2Color_);
  printer->CloseElement();

  // Console color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Console");
  printer->PushAttribute("value", consoleColor_);
  printer->CloseElement();

  // Cursor color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Cursor");
  printer->PushAttribute("value", cursorColor_);
  printer->CloseElement();

  // Info color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Info");
  printer->PushAttribute("value", infoColor_);
  printer->CloseElement();

  // Warning color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Warning");
  printer->PushAttribute("value", warnColor_);
  printer->CloseElement();

  // Error color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Error");
  printer->PushAttribute("value", errorColor_);
  printer->CloseElement();

  // Play color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Play");
  printer->PushAttribute("value", playColor_);
  printer->CloseElement();

  // Mute color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Mute");
  printer->PushAttribute("value", muteColor_);
  printer->CloseElement();

  // SongViewFE color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "SongViewFE");
  printer->PushAttribute("value", songViewFEColor_);
  printer->CloseElement();

  // SongView00 color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "SongView00");
  printer->PushAttribute("value", songView00Color_);
  printer->CloseElement();

  // Row color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Row");
  printer->PushAttribute("value", rowColor_);
  printer->CloseElement();

  // Row2 color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "Row2");
  printer->PushAttribute("value", row2Color_);
  printer->CloseElement();

  // MajorBeat color
  printer->OpenElement("Color");
  printer->PushAttribute("name", "MajorBeat");
  printer->PushAttribute("value", majorBeatColor_);
  printer->CloseElement();

  printer->CloseElement(); // Colors
}

void Theme::RestoreContent(PersistencyDocument *doc) {
  Trace::Log("THEME", "Restoring theme content from XML");

  // Set default name in case we don't find one
  name_ = "Imported Theme";

  // Navigate through the XML document to find our elements
  if (doc->FirstChild()) {
    do {
      char *elemName = doc->ElemName();

      if (strcmp(elemName, "Name") == 0) {
        // Process Name element
        if (doc->HasContent()) {
          name_ = doc->content_;
        }
      } else if (strcmp(elemName, "Font") == 0) {
        // Process Font element
        while (doc->NextAttribute()) {
          if (strcmp(doc->attrname_, "value") == 0) {
            fontValue_ = atoi(doc->attrval_);
          }
        }
      } else if (strcmp(elemName, "Colors") == 0) {
        // Process Colors element
        if (doc->FirstChild()) {
          do {
            if (strcmp(doc->ElemName(), "Color") == 0) {
              // Process Color element
              const char *colorName = nullptr;
              int colorValue = 0;

              while (doc->NextAttribute()) {
                if (strcmp(doc->attrname_, "name") == 0) {
                  colorName = doc->attrval_;
                } else if (strcmp(doc->attrname_, "value") == 0) {
                  colorValue = atoi(doc->attrval_);
                }
              }

              if (colorName) {
                if (strcmp(colorName, "Background") == 0) {
                  bgColor_ = colorValue;
                } else if (strcmp(colorName, "Foreground") == 0) {
                  fgColor_ = colorValue;
                } else if (strcmp(colorName, "Highlight1") == 0) {
                  hi1Color_ = colorValue;
                } else if (strcmp(colorName, "Highlight2") == 0) {
                  hi2Color_ = colorValue;
                } else if (strcmp(colorName, "Console") == 0) {
                  consoleColor_ = colorValue;
                } else if (strcmp(colorName, "Cursor") == 0) {
                  cursorColor_ = colorValue;
                } else if (strcmp(colorName, "Info") == 0) {
                  infoColor_ = colorValue;
                } else if (strcmp(colorName, "Warning") == 0) {
                  warnColor_ = colorValue;
                } else if (strcmp(colorName, "Error") == 0) {
                  errorColor_ = colorValue;
                } else if (strcmp(colorName, "Play") == 0) {
                  playColor_ = colorValue;
                } else if (strcmp(colorName, "Mute") == 0) {
                  muteColor_ = colorValue;
                } else if (strcmp(colorName, "SongViewFE") == 0) {
                  songViewFEColor_ = colorValue;
                } else if (strcmp(colorName, "SongView00") == 0) {
                  songView00Color_ = colorValue;
                } else if (strcmp(colorName, "Row") == 0) {
                  rowColor_ = colorValue;
                } else if (strcmp(colorName, "Row2") == 0) {
                  row2Color_ = colorValue;
                } else if (strcmp(colorName, "MajorBeat") == 0) {
                  majorBeatColor_ = colorValue;
                }
              }
            }
          } while (doc->NextSibling());
        }
      }
    } while (doc->NextSibling());
  }
}
