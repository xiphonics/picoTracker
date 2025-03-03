#ifndef _PROJECT_VIEW_H_
#define _PROJECT_VIEW_H_

#include "BaseClasses/UITextField.h"
#include "FieldView.h"
#include "Foundation/Observable.h"
#include "ViewData.h"
#include <stdint.h>

template <uint8_t MaxLength> class UITextField;

class ProjectView : public FieldView, public I_Observer {
public:
  ProjectView(GUIWindow &w, ViewData *data);
  virtual ~ProjectView();

  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};
  virtual void OnFocus();
  virtual void AnimationUpdate();
  etl::string<MAX_PROJECT_NAME_LENGTH> getProjectName() {
    return nameField_->GetString();
  };

  etl::string<MAX_PROJECT_NAME_LENGTH> getOldProjectName() {
    return oldProjName_;
  };
  void clearSaveAsFlag() {
    saveAsFlag_ = false;
    oldProjName_ = getProjectName();
  };

  // Observer for action callback

  void Update(Observable &, I_ObservableData *);

  void OnPurgeInstruments(bool removeFromDisk);
  void OnQuit();

private:
  Project *project_;
  // Debug
  unsigned long lastTick_;
  unsigned long lastClock_;
  UIField *tempoField_;
  UITextField<MAX_PROJECT_NAME_LENGTH> *nameField_;
  bool saveAsFlag_ = false;
  etl::string<MAX_PROJECT_NAME_LENGTH> oldProjName_;
};
#endif
