/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Song.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Utils/HexBuffers.h"
#include "Phrase.h"
#include "System/System/System.h"
#include "System/io/Status.h"
#include "Table.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Song::Song() : Persistent("SONG"), chain_(), phrase_() {

  for (int i = 0; i < SONG_CHANNEL_COUNT * SONG_ROW_COUNT; i++)
    data_[i] = EMPTY_SONG_VALUE;
};

Song::~Song(){};

void Song::SaveContent(tinyxml2::XMLPrinter *printer) {
  saveHexBuffer(printer, "SONG", data_, SONG_ROW_COUNT * SONG_CHANNEL_COUNT);
  saveHexBuffer(printer, "CHAINS", chain_.data_,
                CHAIN_COUNT * PHRASES_PER_CHAIN);
  saveHexBuffer(printer, "TRANSPOSES", chain_.transpose_,
                CHAIN_COUNT * PHRASES_PER_CHAIN);
  saveHexBuffer(printer, "NOTES", phrase_.note_,
                PHRASE_COUNT * STEPS_PER_PHRASE);
  saveHexBuffer(printer, "INSTRUMENTS", phrase_.instr_,
                PHRASE_COUNT * STEPS_PER_PHRASE);
  saveHexBuffer(printer, "COMMAND1", phrase_.cmd1_,
                PHRASE_COUNT * STEPS_PER_PHRASE);
  saveHexBuffer(printer, "PARAM1", phrase_.param1_,
                PHRASE_COUNT * STEPS_PER_PHRASE);
  saveHexBuffer(printer, "COMMAND2", phrase_.cmd2_,
                PHRASE_COUNT * STEPS_PER_PHRASE);
  saveHexBuffer(printer, "PARAM2", phrase_.param2_,
                PHRASE_COUNT * STEPS_PER_PHRASE);
};

void Song::RestoreContent(PersistencyDocument *doc) {
  bool elem = doc->FirstChild();

  while (elem) {
    if (!strcmp("SONG", doc->ElemName())) {
      restoreHexBuffer(doc, data_);
    };
    if (!strcmp("CHAINS", doc->ElemName())) {
      restoreHexBuffer(doc, chain_.data_);
    };
    if (!strcmp("TRANSPOSES", doc->ElemName())) {
      restoreHexBuffer(doc, chain_.transpose_);
    };
    if (!strcmp("NOTES", doc->ElemName())) {
      restoreHexBuffer(doc, phrase_.note_);
    };
    if (!strcmp("INSTRUMENTS", doc->ElemName())) {
      restoreHexBuffer(doc, phrase_.instr_);
    };
    if (!strcmp("COMMAND1", doc->ElemName())) {
      restoreHexBuffer(doc, (uchar *)phrase_.cmd1_);
    };
    if (!strcmp("PARAM1", doc->ElemName())) {
      restoreHexBuffer(doc, (uchar *)phrase_.param1_);
    };
    if (!strcmp("COMMAND2", doc->ElemName())) {
      restoreHexBuffer(doc, (uchar *)phrase_.cmd2_);
    };
    if (!strcmp("PARAM2", doc->ElemName())) {
      restoreHexBuffer(doc, (uchar *)phrase_.param2_);
    };
    elem = doc->NextSibling();
  }

  Status::Set("Restoring allocation");

  // Restore chain & phrase allocation table

  unsigned char *data = data_;
  for (int i = 0; i < SONG_ROW_COUNT * SONG_CHANNEL_COUNT; i++) {
    if (*data != 0xFF) {
      if (*data < 0x80) {
        chain_.SetUsed(*data);
      }
    }
    data++;
  }

  data = chain_.data_;

  for (int i = 0; i < CHAIN_COUNT; i++) {
    for (int j = 0; j < 16; j++) {
      if (*data != 0xFF) {
        chain_.SetUsed(i);
        phrase_.SetUsed(*data);
      }
      data++;
    };
  }

  data = phrase_.note_;

  FourCC *table1 = phrase_.cmd1_;
  FourCC *table2 = phrase_.cmd2_;

  ushort *param1 = phrase_.param1_;
  ushort *param2 = phrase_.param2_;

  TableHolder *th = TableHolder::GetInstance();

  for (int i = 0; i < PHRASE_COUNT; i++) {
    for (int j = 0; j < 16; j++) {
      if (*data != 0xFF) {
        phrase_.SetUsed(i);
      }
      if (*table1 == FourCC::InstrumentCommandTable) {
        *param1 &= 0x7F;
        th->SetUsed((*param1));
      };
      if (*table2 == FourCC::InstrumentCommandTable) {
        *param2 &= 0x7F;
        th->SetUsed((*param2));
      };
      table1++;
      table2++;
      param1++;
      param2++;
      data++;
    };
  }
};
