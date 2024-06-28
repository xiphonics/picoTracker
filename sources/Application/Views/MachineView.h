#ifndef _MACHINE_VIEW_H_
#define _MACHINE_VIEW_H_

#include "BaseClasses/FieldView.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIIntVarField.h"
#include "Foundation/Observable.h"
#include "ViewData.h"

class MachineView : public FieldView, public I_Observer {
public:
  MachineView(GUIWindow &w, ViewData *data);
  virtual ~MachineView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus(){};
  virtual void AnimationUpdate(){};

  // Observer for action callback

  void Update(Observable &, I_ObservableData *);

protected:
private:
  etl::vector<UIIntVarField, 1> intVarField_;
  etl::vector<UIActionField, 1> actionField_;
};
#endif
