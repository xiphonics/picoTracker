/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SONG_H_
#define _SONG_H_

#include "Application/Persistency/Persistent.h"
#include "Chain.h"
#include "Phrase.h"

#define SONG_CHANNEL_COUNT 8
#define SONG_ROW_COUNT 128

#define MAX_INSTRUMENT_COUNT 32

#define HIGHEST_NOTE 119
#define NOTE_OFF 0xFE
#define NO_NOTE 0xFF
#define NOTE_C3 60
#define EMPTY_SONG_VALUE 0xFF

class Song : Persistent {
public:
  Song();
  ~Song();
  void Reset();

  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

  unsigned char data_[SONG_CHANNEL_COUNT * SONG_ROW_COUNT];
  Chain chain_;
  Phrase phrase_;
};

#endif
