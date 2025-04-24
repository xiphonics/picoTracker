#ifndef _THEME_VIEW_H_
#define _THEME_VIEW_H_

#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UISwatchField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
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

protected:
private:
  void addSwatchField(ColorDefinition color, GUIPoint position);

  etl::vector<UIIntVarField, 2> intVarField_;
  etl::vector<UIBigHexVarField, 16>
      bigHexVarField_;                         // Increased for all color fields
  etl::vector<UISwatchField, 16> swatchField_; // Increased for all color fields
  etl::vector<UIActionField, 1> actionField_;  // For "Back" button
};

#endif
