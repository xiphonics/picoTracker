#include "TextInputModalView.h"
#include "Application/AppWindow.h"
#include "MessageBox.h" // Include for button constants

TextInputModalView::TextInputModalView(
    View &view, const char *title, const char *prompt, int btnFlags,
    etl::string<MAX_TEXT_INPUT_LENGTH> defaultValue)
    : ModalView(view), title_(title), prompt_(prompt),
      textVariable_(FourCC::ActionEdit, defaultValue.c_str()) {

  editingText_ = true;
  focus_ = nullptr;

  buttonCount_ = 0;
  for (int i = 0; i < MBL_LAST; i++) {
    if (btnFlags & (1 << (i))) {
      button_[buttonCount_] = i;
      buttonCount_++;
    }
  }
  selected_ = buttonCount_ - 1;
  NAssert(buttonCount_ != 0);

  GUIPoint position = GetAnchor();
  position._y += 2; // 2 lines below title
  position._x += 6; // After "Name: " prompt

  textField_ = new UITextField<MAX_TEXT_INPUT_LENGTH>(
      textVariable_, position, "", FourCC::ActionEdit, defaultValue);
  textField_->AddObserver(*this);

  // Position the button
  position._y += 2; // 2 lines below text field
  position._x += 1; // Align roughly in middle of the dialog

  // Set initial focus to text field
  SetFocus(textField_);
}

TextInputModalView::~TextInputModalView() {}

void TextInputModalView::DrawView() {
  // Calculate window size based on title and prompt
  int titleSize = title_.size();
  int promptSize = 6 + MAX_TEXT_INPUT_LENGTH; // "Name: " + text field length
  
  // compute space needed for buttons
  int btnSize = 5; // Average button text size
  int buttonWidth = buttonCount_ * (btnSize + 1) + 1;
  
  int width = titleSize > promptSize ? titleSize : promptSize;
  width = width > buttonWidth ? width : buttonWidth;
  
  SetWindow(width, 5);

  // Draw title
  GUITextProperties props;
  props.invert_ = true;
  SetColor(CD_NORMAL);
  int x = (width - titleSize) / 2;
  DrawString(x, 0, title_.c_str(), props);

  // Draw prompt
  props.invert_ = false;
  x = 0;
  DrawString(x, 2, prompt_.c_str(), props);

  // Draw text field
  if (focus_ == textField_) {
    SetColor(CD_HILITE1);
  } else {
    SetColor(CD_NORMAL);
  }
  textField_->Draw(w_);

  // Draw buttons (similar to MessageBox)
  SetColor(CD_NORMAL);
  int y = 4; // Position for buttons
  int offset = width / (buttonCount_ + 1);

  static const char *buttonText[MBL_LAST] = {"Ok", "Yes", "Cancel", "No"};
  
  for (int i = 0; i < buttonCount_; i++) {
    const char *text = buttonText[button_[i]];
    x = offset * (i + 1) - strlen(text) / 2;
    props.invert_ = (focus_ == nullptr && i == selected_) ? true : false;
    DrawString(x, y, text, props);
  }
  // Update editingText_ flag based on focus for backward compatibility
  editingText_ = (focus_ == textField_);
}

void TextInputModalView::OnFocus() {}

void TextInputModalView::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (!pressed) {
    return;
  }

  if (focus_ == textField_) {
    // Handle text field input
    if (mask & EPBM_ENTER) {
      // textfields procesarrow expects UP/DOWN/LEFT/RIGHT value not a mask
      // so need to do that processing here
      if (mask & EPBM_UP) {
        textField_->ProcessArrow(EPBM_UP);
      } else if (mask & EPBM_DOWN) {
        textField_->ProcessArrow(EPBM_DOWN);
      } else if (mask & EPBM_LEFT) {
        textField_->ProcessArrow(EPBM_LEFT);
      } else if (mask & EPBM_RIGHT) {
        textField_->ProcessArrow(EPBM_RIGHT);
      } else {
        textField_->OnEditClick();
      }
    } else if (mask & EPBM_EDIT || mask & EPBM_DOWN) {
      // Switch focus to buttons
      focus_ = nullptr;
    }
  } else {
    // Button selection mode
    if (mask & EPBM_LEFT) {
      selected_ = (selected_ + 1);
      if (selected_ >= buttonCount_) {
        selected_ = 0;
      }
    } else if (mask & EPBM_RIGHT) {
      selected_ = (selected_ - 1);
      if (selected_ < 0) {
        selected_ = buttonCount_ - 1;
      }
    } else if (mask & EPBM_UP) {
      // Move focus back to text field
      SetFocus(textField_);
    } else if (mask & EPBM_ENTER) {
      EndModal(button_[selected_]);
    }
  }
  isDirty_ = true;
}

// Focus management implementation
void TextInputModalView::SetFocus(UIField *field) {
  if (focus_) {
    focus_->ClearFocus();
  }
  focus_ = field;
  if (focus_) {
    focus_->SetFocus();
    // Update editingText_ flag for backward compatibility
    editingText_ = (focus_ == textField_);
  }
}

void TextInputModalView::ClearFocus() {
  if (focus_) {
    focus_->ClearFocus();
  }
  focus_ = nullptr;
}

// Implement the Update method for observer pattern
void TextInputModalView::Update(Observable &o, I_ObservableData *d) {
  // Handle notifications from text field
  isDirty_ = true;
}