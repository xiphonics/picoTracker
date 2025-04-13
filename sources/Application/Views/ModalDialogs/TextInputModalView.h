#ifndef _TEXT_INPUT_MODAL_VIEW_H_
#define _TEXT_INPUT_MODAL_VIEW_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "Application/Views/BaseClasses/UIActionField.h"
#include "Application/Views/BaseClasses/UITextField.h"
#include "Externals/etl/include/etl/string.h"
#include "Foundation/Observable.h"
#include "Foundation/Variables/Variable.h"

#define MAX_TEXT_TITLE_LENGTH 24
#define MAX_TEXT_PROMPT_LENGTH 8
#define MAX_TEXT_INPUT_LENGTH (MAX_TEXT_TITLE_LENGTH - MAX_TEXT_PROMPT_LENGTH)

enum TextInputButtonFlag { TIB_OK = 1, TIB_CANCEL = 2 };

class TextInputModalView : public ModalView, public I_Observer {
public:
  TextInputModalView(View &view, const char *title, const char *prompt,
                     const char *buttonLabel,
                     etl::string<MAX_TEXT_INPUT_LENGTH> defaultValue);
  virtual ~TextInputModalView();

  virtual void DrawView();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void OnFocus();
  virtual void AnimationUpdate(){};
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int){};

  // Observer implementation
  virtual void Update(Observable &o, I_ObservableData *d);

  UITextField<MAX_TEXT_INPUT_LENGTH> *GetTextField() { return textField_; }

  // Focus management methods
  void SetFocus(UIField *field);
  UIField *GetFocus() { return focus_; }
  void ClearFocus();

private:
  etl::string<MAX_TEXT_TITLE_LENGTH> title_;
  etl::string<MAX_TEXT_PROMPT_LENGTH> prompt_;
  Variable textVariable_;
  UITextField<MAX_TEXT_INPUT_LENGTH> *textField_;
  UIActionField *okButton_;
  UIField *focus_; // Current focused field
  bool editingText_;
};

#endif