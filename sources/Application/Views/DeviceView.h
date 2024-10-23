#ifndef _DEVICE_VIEW_H_
#define _DEVICE_VIEW_H_

#include "BaseClasses/FieldView.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UISwatchField.h"
#include "Foundation/Observable.h"
#include "ViewData.h"

class DeviceView : public FieldView, public I_Observer {
public:
  DeviceView(GUIWindow &w, ViewData *data);
  virtual ~DeviceView();

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

  etl::vector<UIIntVarField, 5> intVarField_;
  etl::vector<UIActionField, 1> actionField_;
  etl::vector<UIBigHexVarField, 9> bigHexVarField_;
  etl::vector<UISwatchField, 9> swatchField_;
};
#endif
