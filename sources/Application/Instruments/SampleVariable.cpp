/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleVariable.h"
#include "SamplePool.h"

SampleVariable::SampleVariable(FourCC id) : WatchedVariable(id, 0, 0, -1) {
  SamplePool *pool = SamplePool::GetInstance();
  list_.char_ = pool->GetNameList();
  listSize_ = pool->GetNameListSize();
  pool->AddObserver(*this);
};

SampleVariable::~SampleVariable() {
  SamplePool *pool = SamplePool::GetInstance();
  pool->RemoveObserver(*this);
};

void SampleVariable::Update(Observable &o, I_ObservableData *d) {
  SamplePoolEvent *e = (SamplePoolEvent *)d;
  // If a sample was removed, update our index and notify observers so
  // instruments retarget their sample pointer.
  if (e->type_ == SPET_DELETE) {
    int currentIndex = value_.index_;
    int newIndex = currentIndex;

    if (currentIndex == e->index_) {
      newIndex = -1; // sample deleted, clear selection
    } else if (currentIndex > e->index_) {
      newIndex = currentIndex - 1;
    }

    if (newIndex != currentIndex) {
      SetInt(newIndex); // triggers onChange/NotifyObservers
    }
  }
  // For inserts, just refresh list pointers below
  // indices remain valid since imports append at the end
  SamplePool *pool = (SamplePool *)&o;
  list_.char_ = pool->GetNameList();
  listSize_ = pool->GetNameListSize();
};
