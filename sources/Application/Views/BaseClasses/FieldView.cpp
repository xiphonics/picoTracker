#include "FieldView.h"
#include "System/Console/Trace.h"
#include "UIIntVarField.h"

FieldView::FieldView(GUIWindow &w, ViewData *data) : View(w, data) {
  focus_ = 0;
};

void FieldView::SetFocus(UIField *field) {

  if (focus_) {
    focus_->ClearFocus();
  }
  focus_ = field;

  //  Empty field view, we don't have anything to do

  if (focus_ == 0)
    return;

  focus_->SetFocus();
};

void FieldView::ClearFocus() {
  if (focus_) {
    focus_->ClearFocus();
  };
  focus_ = 0;
};

UIField *FieldView::GetFocus() { return focus_; };

void FieldView::Redraw() {

  if (focus_ == 0) {
    SetFocus(*fieldList_.begin());
  }

  auto it = fieldList_.begin();
  for (size_t i = 0; i < fieldList_.size(); i++) {
    (*it)->Draw(w_);
    it++;
  };
};

void FieldView::ProcessButtonMask(unsigned short mask) {

  if (focus_ == 0) {
    focus_ = *fieldList_.begin();
    //  Empty field view, we don't have anything to do
    if (focus_ == 0)
      return;
    focus_->SetFocus();
  }

  if (mask & EPBM_A) { // A or A+ARROW is sent to the field
    if (mask & EPBM_DOWN) {
      focus_->ProcessArrow(EPBM_DOWN);
      isDirty_ = true;
    }
    if (mask & EPBM_UP) {
      focus_->ProcessArrow(EPBM_UP);
      isDirty_ = true;
    }

    if (mask & EPBM_LEFT) {
      focus_->ProcessArrow(EPBM_LEFT);
      isDirty_ = true;
    }

    if (mask & EPBM_RIGHT) {
      focus_->ProcessArrow(EPBM_RIGHT);
      isDirty_ = true;
    }

    if (mask == EPBM_A) {
      focus_->OnClick();
    };

  } else {
    if (mask & EPBM_B) { // B or B+ARROW is sent to the field

      if (mask == EPBM_B) {
        focus_->OnBClick();
        isDirty_ = true;
      };

      if (mask & EPBM_DOWN) {
        focus_->ProcessBArrow(EPBM_DOWN);
        isDirty_ = true;
      }
      if (mask & EPBM_UP) {
        focus_->ProcessBArrow(EPBM_UP);
        isDirty_ = true;
      }

      if (mask & EPBM_LEFT) {
        focus_->ProcessBArrow(EPBM_LEFT);
        isDirty_ = true;
      }

      if (mask & EPBM_RIGHT) {
        focus_->ProcessBArrow(EPBM_RIGHT);
        isDirty_ = true;
      }

    } else { // Nor B or A is pressed

      if (!(mask &
            (EPBM_A | EPBM_B | EPBM_L | EPBM_R | EPBM_SELECT | EPBM_START))) {

        if (mask & EPBM_DOWN) {
          UIField *next = 0;
          UIField *first = 0;

          auto it = fieldList_.begin();
          for (size_t i = 0; i < fieldList_.size(); i++) {
            if (!(*it)->IsStatic()) {
              if (first) {
                if ((*it)->GetPosition()._y < first->GetPosition()._y) {
                  first = *it;
                };
              } else {
                first = *it;
              }
              if ((*it)->GetPosition()._y > focus_->GetPosition()._y) {
                if (next) {
                  if ((*it)->GetPosition()._y < next->GetPosition()._y) {
                    next = *it;
                  } else {
                    // if both target at same height
                  };
                } else {
                  next = *it;
                };
              };
            }
            it++;
          }
          if (next == 0) {
            next = first;
          }

          focus_->ClearFocus();
          focus_ = next;
          focus_->SetFocus();
          isDirty_ = true;
        }

        if (mask & EPBM_UP) {

          UIField *prev = 0;
          UIField *last = 0;

          auto it = fieldList_.begin();
          for (size_t i = 0; i < fieldList_.size(); i++) {

            if (!(*it)->IsStatic()) {
              if (last) {
                if ((*it)->GetPosition()._y > last->GetPosition()._y) {
                  last = *it;
                };
              } else {
                last = *it;
              }
              if ((*it)->GetPosition()._y < focus_->GetPosition()._y) {
                if (prev) {
                  if ((*it)->GetPosition()._y > prev->GetPosition()._y) {
                    prev = *it;
                  } else {
                    // if both target at same height
                  };
                } else {
                  prev = *it;
                };
              };
            }
            it++;
          }
          if (prev == 0) {
            prev = last;
          }

          focus_->ClearFocus();
          focus_ = prev;
          focus_->SetFocus();
          isDirty_ = true;
        }

        if (mask & EPBM_RIGHT) {
          UIField *next = 0;
          UIField *first = 0;

          auto it = fieldList_.begin();
          for (size_t i = 0; i < fieldList_.size(); i++) {

            if (!(*it)->IsStatic() &&
                ((*it)->GetPosition()._y == focus_->GetPosition()._y)) {
              if (first) {
                if ((*it)->GetPosition()._x < first->GetPosition()._x) {
                  first = *it;
                };
              } else {
                first = *it;
              }
              if ((*it)->GetPosition()._x > focus_->GetPosition()._x) {
                if (next) {
                  if ((*it)->GetPosition()._x < next->GetPosition()._x) {
                    next = *it;
                  } else {
                    // if both target at same height
                  };
                } else {
                  next = *it;
                };
              };
            }
            it++;
          }
          if (next == 0) {
            next = first;
          }

          focus_->ClearFocus();
          focus_ = next;
          focus_->SetFocus();
          isDirty_ = true;
        }

        if (mask & EPBM_LEFT) {

          UIField *prev = 0;
          UIField *last = 0;

          auto it = fieldList_.begin();
          for (size_t i = 0; i < fieldList_.size(); i++) {

            if (!(*it)->IsStatic() &&
                ((*it)->GetPosition()._y == focus_->GetPosition()._y)) {
              if (last) {
                if ((*it)->GetPosition()._x > last->GetPosition()._x) {
                  last = *it;
                };
              } else {
                last = *it;
              }
              if ((*it)->GetPosition()._x < focus_->GetPosition()._x) {
                if (prev) {
                  if ((*it)->GetPosition()._x > prev->GetPosition()._x) {
                    prev = *it;
                  } else {
                    // if both target at same height
                  };
                } else {
                  prev = *it;
                };
              };
            }
            it++;
          }
          if (prev == 0) {
            prev = last;
          }

          focus_->ClearFocus();
          focus_ = prev;
          focus_->SetFocus();
          isDirty_ = true;
        }
      }
    }
  }
}

int FieldView::GetFocusIndex() {

  int focusIndex = 0;
  auto it = fieldList_.begin();
  for (size_t i = 0; i < fieldList_.size(); i++) {
    if (*it == focus_) {
      break;
    };
    focusIndex++;
    it++;
  };
  return focusIndex;
}
