#ifndef _THEME_VIEW_H_
#define _THEME_VIEW_H_

#include "Application/Model/Config.h"
#include "Application/Model/Theme.h"
#include "Application/Views/BaseClasses/ModalView.h"
#include "Application/Views/UIController.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UISwatchField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "Foundation/T_SimpleList.h"
#include "Foundation/T_Stack.h"
#include "ViewData.h"

class ThemeView : public FieldView, public I_Observer {
public:
  ThemeView(GUIWindow &w, ViewData *data);
  virtual ~ThemeView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus(){};
  virtual void AnimationUpdate(){};

  // Observer for action callback
  void Update(Observable &, I_ObservableData *);

  // For storing export theme name during modal callbacks
  etl::string<MAX_INSTRUMENT_NAME_LENGTH> exportThemeName_;

protected:
private:
  void addSwatchField(ColorDefinition color, GUIPoint position);

  etl::vector<UIIntVarField, 2> intVarField_;
  etl::vector<UIBigHexVarField, 16> bigHexVarField_;
  etl::vector<UISwatchField, 16> swatchField_;
  etl::vector<UIActionField, 2> actionField_; // For Import/Export buttons

  // Helper methods for theme import/export
  void handleThemeExport();
  void exportTheme();
  void importTheme();
};
#endif
