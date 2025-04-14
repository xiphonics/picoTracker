#include "TextInputModalView.h"
#include "Application/AppWindow.h"

TextInputModalView::TextInputModalView(
    View &view, const char *title, const char *prompt, const char *buttonLabel,
    etl::string<MAX_TEXT_INPUT_LENGTH> defaultValue)
    : ModalView(view), title_(title), prompt_(prompt),
      textVariable_(FourCC::ActionEdit, defaultValue.c_str()) {

  editingText_ = true;
  focus_ = nullptr;

  GUIPoint position = GetAnchor();
  position._y += 2; // 2 lines below title
  position._x += 6; // After "Name: " prompt

  textField_ = new UITextField<MAX_TEXT_INPUT_LENGTH>(
      textVariable_, position, "", FourCC::ActionEdit, defaultValue);
  textField_->AddObserver(*this);

  // Position the button
  position._y += 2; // 2 lines below text field
  position._x += 1; // Align roughly in middle of the dialog

  okButton_ = new UIActionField(buttonLabel, FourCC::ActionOK, position);
  okButton_->AddObserver(*this);

  // Set initial focus to text field
  SetFocus(textField_);
}

TextInputModalView::~TextInputModalView() {}

void TextInputModalView::DrawView() {
  // Calculate window size based on title and prompt
  int titleSize = title_.size();
  int promptSize = 6 + MAX_TEXT_INPUT_LENGTH; // "Name: " + text field length
  int width = std::max(titleSize, promptSize);
  width = width > 20 ? width : 20; // Ensure minimum width

  // Set window size (width, height)
  SetWindow(width, 4); // Height of 4 for: title + text field + button + margin

  // Draw title centered at the top
  GUITextProperties titleProps;
  SetColor(CD_NORMAL);
  DrawString((width - title_.size()) / 2, 0, title_.c_str(), titleProps);

  // Draw "Name: " label on second line
  GUITextProperties promptProps;
  SetColor(CD_NORMAL);
  // 2 lines below the title
  DrawString(0, 2, "Name: ", promptProps);

  // The text field and button will draw themselves based on their positions
  textField_->Draw(w_);
  okButton_->Draw(w_);

  // Update editingText_ flag based on focus for backward compatibility
  editingText_ = (focus_ == textField_);
}

void TextInputModalView::OnFocus() {}

void TextInputModalView::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed) {
    return;
  }

  // handle nav + left to cancel and leave dialog
  if (mask & EPBM_LEFT) {
    EndModal(TIB_CANCEL);
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
      // Switch focus to button
      SetFocus(okButton_);
    }
  } else if (focus_ == okButton_) {
    // Handle button selection
    if (mask & EPBM_UP) {
      // Move focus back to text field
      SetFocus(textField_);
    } else if (mask & EPBM_ENTER || mask & EPBM_EDIT) {
      EndModal(TIB_OK);
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
  // Handle notifications from fields if needed
  isDirty_ = true;
}