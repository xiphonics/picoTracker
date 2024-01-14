#include "PersistencyDocument.h"

PersistencyDocument::PersistencyDocument() {
  version_ = 0;
  yxml_init(state_, stack_, sizeof(stack_));
  r_ = YXML_OK; // initialize to ok value
}

bool PersistencyDocument::Load(const std::string &filename) {
  fp_ = FileSystem::GetInstance()->Open(filename.c_str(), "r");
  if (fp_)
    return true;
  return false;
}

char *PersistencyDocument::ElemName() { return state_->elem; }

bool PersistencyDocument::FirstChild() {
  // Only to be called after YXML_ATTREND or YXML_ELEMSTART or YXML_CONTENT
  if ((r_ != YXML_OK) && (r_ != YXML_ELEMSTART) && (r_ != YXML_ATTREND) &&
      (r_ != YXML_CONTENT)) {
    return false;
  }

  int c;
  while ((c = fp_->GetC()) != EOF) {
    r_ = yxml_parse(state_, c);
    switch (r_) {
    case YXML_ELEMSTART:
      return true;
    case YXML_ELEMEND:
      return false;
    case YXML_CONTENT:
    case YXML_ATTRSTART:
    case YXML_ATTRVAL:
    case YXML_ATTREND:
    case YXML_EEOF:
    case YXML_EREF:
    case YXML_ECLOSE:
    case YXML_ESTACK:
    case YXML_ESYN:
      // Error
    default:
      // Any other values we skip, including YXML_OK
      break;
    }
  }
  return false;
}

bool PersistencyDocument::NextSibling() {
  // Only to be called after YXML_ELEMEND
  if ((r_ != YXML_OK) && (r_ != YXML_ELEMEND))
    return false;
  int c;
  while ((c = fp_->GetC()) != EOF) {
    r_ = yxml_parse(state_, c);
    switch (r_) {
    case YXML_ELEMSTART:
      return true;
    case YXML_ELEMEND:
      return false;
    case YXML_CONTENT:
    case YXML_ATTRSTART:
    case YXML_ATTRVAL:
    case YXML_ATTREND:
    case YXML_EEOF:
    case YXML_EREF:
    case YXML_ECLOSE:
    case YXML_ESTACK:
    case YXML_ESYN:
      // Error
    default:
      // Any other values we skip, including YXML_OK
      break;
    }
  }
  return false;
}

bool PersistencyDocument::NextAttribute() {
  // This is called as soon as a YXML_ELEMSTART happened or after another
  // YXML_ATTREND
  if ((r_ != YXML_OK) && (r_ != YXML_ELEMSTART) && (r_ != YXML_ATTREND))
    return false;

  int cur = 0;
  int c;
  while ((c = fp_->GetC()) != EOF) {
    r_ = yxml_parse(state_, c);
    switch (r_) {
    case YXML_ELEMSTART:
      return false;
      break;
    case YXML_ELEMEND:
      return false;
      break;
    case YXML_CONTENT:
      return false;
      break;
    case YXML_ATTRSTART:
      strcpy(attrname_, state_->attr);
      break;
    case YXML_ATTRVAL:
      attrval_[cur] = state_->data[0];
      cur++;
      break;
    case YXML_ATTREND:
      attrval_[cur] = '\0';
      return true;
      break;
    case YXML_EEOF:
    case YXML_EREF:
    case YXML_ECLOSE:
    case YXML_ESTACK:
    case YXML_ESYN:
      // Error
    default:
      // Any other values we skip, including YXML_OK
      break;
    }
  }
  return false;
}

bool PersistencyDocument::HasContent() {
  // This is called after YXML_ELEMSTART, YXML_ATTREND or YXML_CONTENT
  if ((r_ != YXML_OK) && (r_ != YXML_ELEMSTART) && (r_ != YXML_ATTREND) &&
      (r_ != YXML_CONTENT))
    return false;

  bool found = false;
  int cur = 0;
  int c;

  // if YXML_CONTENT happened before reaching here, there is already one
  // character in the buffer
  if (r_ == YXML_CONTENT) {
    content_[cur] = state_->data[0];
    cur++;
  }

  while ((c = fp_->GetC()) != EOF) {
    r_ = yxml_parse(state_, c);
    switch (r_) {
    case YXML_ELEMSTART:
      return false;
      break;
    case YXML_ELEMEND:
      if (found) {
        content_[cur] = '\0';
        return true;
      }
      return false;
      break;
    case YXML_CONTENT:
      content_[cur] = state_->data[0];
      cur++;
      found = true;
      break;
    case YXML_ATTRSTART:
    case YXML_ATTRVAL:
    case YXML_ATTREND:
    case YXML_EEOF:
    case YXML_EREF:
    case YXML_ECLOSE:
    case YXML_ESTACK:
    case YXML_ESYN:
      // Error
    default:
      // Any other values we skip, including YXML_OK
      break;
    }
  }
  return false;
}
