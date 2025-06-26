/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "PersistencyDocument.h"
#include "System/Console/Trace.h"

PersistencyDocument::PersistencyDocument() {
  version_ = 0;
  yxml_init(state_, stack_, sizeof(stack_));
  r_ = YXML_OK;  // initialize to ok value
  fp_ = nullptr; // Initialize file pointer to null
}

PersistencyDocument::~PersistencyDocument() {
  // Ensure file is closed when object is destroyed
  Close();
}

void PersistencyDocument::Close() {
  if (fp_) {
    fp_->Close();
    fp_ = nullptr;
    Trace::Log("PERSISTENCYDOCUMENT", "File closed");
  }
}

bool PersistencyDocument::Load(const char *filename) {
  Trace::Log("PERSISTENCYDOCUMENT", "Loading document from file: %s", filename);

  fp_ = FileSystem::GetInstance()->Open(filename, "r");
  if (!fp_) {
    Trace::Error("Failed to open file: %s", filename);
    return false;
  }

  // Reset the XML parser state
  yxml_init(state_, stack_, sizeof(stack_));
  r_ = YXML_OK;

  // Verify we can read from the file
  int c = fp_->GetC();
  if (c == EOF) {
    Trace::Error("File is empty or cannot be read: %s", filename);
    fp_->Close();
    fp_ = nullptr;
    return false;
  }

  // Reset file position
  fp_->Seek(0, SEEK_SET);

  Trace::Log("PERSISTENCYDOCUMENT", "Successfully opened file: %s", filename);
  return true;
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
  if ((r_ != YXML_OK) && (r_ != YXML_ELEMEND)) {
    Trace::Error(
        "XML NextSibling called with invalid state: %d for element '%s'", r_,
        state_->elem ? state_->elem : "<unknown>");

    // Print additional debug info about the parser state
    const char *stateStr = "unknown";
    switch (r_) {
    case YXML_OK:
      stateStr = "YXML_OK";
      break;
    case YXML_ELEMSTART:
      stateStr = "YXML_ELEMSTART";
      break;
    case YXML_CONTENT:
      stateStr = "YXML_CONTENT";
      break;
    case YXML_ELEMEND:
      stateStr = "YXML_ELEMEND";
      break;
    case YXML_ATTRSTART:
      stateStr = "YXML_ATTRSTART";
      break;
    case YXML_ATTRVAL:
      stateStr = "YXML_ATTRVAL";
      break;
    case YXML_ATTREND:
      stateStr = "YXML_ATTREND";
      break;
    case YXML_PISTART:
      stateStr = "YXML_PISTART";
      break;
    case YXML_PICONTENT:
      stateStr = "YXML_PICONTENT";
      break;
    case YXML_PIEND:
      stateStr = "YXML_PIEND";
      break;
    case YXML_EEOF:
      stateStr = "YXML_EEOF";
      break;
    case YXML_EREF:
      stateStr = "YXML_EREF";
      break;
    case YXML_ECLOSE:
      stateStr = "YXML_ECLOSE";
      break;
    case YXML_ESTACK:
      stateStr = "YXML_ESTACK";
      break;
    case YXML_ESYN:
      stateStr = "YXML_ESYN";
      break;
    }
    Trace::Error("XML Parser state: %s, current path: %s", stateStr,
                 state_->elem ? state_->elem : "<none>");
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
      break;
    case YXML_EEOF:
    case YXML_EREF:
    case YXML_ECLOSE:
    case YXML_ESTACK:
    case YXML_ESYN:
      // Error
      Trace::Error("NextSibling encountered error: %d", r_);
      break;
    default:
      // Any other values we skip, including YXML_OK
      break;
    }
  }

  Trace::Error("NextSibling reached EOF");
  return false;
}

bool PersistencyDocument::NextAttribute() {
  // This is called as soon as a YXML_ELEMSTART happened or after another
  // YXML_ATTREND
  if ((r_ != YXML_OK) && (r_ != YXML_ELEMSTART) && (r_ != YXML_ATTREND)) {
    Trace::Error("NextAttribute called with invalid state: %d", r_);
    return false;
  }

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
      Trace::Error("NextAttribute encountered error: %d", r_);
      break;
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
