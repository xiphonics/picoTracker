/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _VIEW_DATA_
#define _VIEW_DATA_

#include "Application/Model/Project.h"
#include "System/Console/Trace.h"

enum PlayMode { PM_SONG, PM_CHAIN, PM_PHRASE, PM_LIVE, PM_AUDITION };

class ViewData {

public:
  ViewData(Project *project);
  ~ViewData();

  unsigned char UpdateSongChain(int offset);
  void UpdateSongOffset(int offset);
  void UpdateSongCursor(int dx, int dy);
  void SetSongChain(unsigned char);
  unsigned char *GetCurrentSongPointer();

  void UpdateChainCursor(int dx, int dy);
  unsigned char UpdateChainCursorValue(int offset, int dx, int dy);
  void SetChainPhrase(unsigned char value);
  unsigned char *GetCurrentChainPointer();

protected:
  void checkSongBoundaries();

  inline void updateData(unsigned char *c, int offset, unsigned char limit,
                         bool wrap) {
    int v = *c;
    if ((v == 0xFF) && (limit != 0xFF)) {
      v = 0;
    }
    v += offset;
    if (v < 0)
      v = (wrap ? (limit + 1 + v) : 0);
    if (v > limit)
      v = (wrap ? v - (limit + 1) : limit);
    *c = v;
  }

public:
  // Data

  Project *project_;
  Song *song_;

  // Editor settings

  int songX_;      // .Current song screen position in the editor
  int songY_;      //
  int songOffset_; // .Current song offset (top screen row) in editor

  int chainRow_; // .Current row in chain editor
  int chainCol_; // .Current column in chain editor

  int currentChain_;  // .Current edited chain
  int currentPhrase_; // .Current edited phrase

  int currentInstrumentID_; // .Current edited instrument

  int currentTable_; // .Current edited table

  int currentGroove_; // .Current edited groove

  // Player Settings

  PlayMode playMode_;
  int songPlayPos_[SONG_CHANNEL_COUNT]; // .Play position of each channel
  unsigned char currentPlayChain_[SONG_CHANNEL_COUNT]; // .Current played chain
                                                       // for each channel
  int chainPlayPos_[SONG_CHANNEL_COUNT]; // .Play position in chain for each
                                         // channel
  unsigned char
      currentPlayPhrase_[SONG_CHANNEL_COUNT]; // .Current played phrase for each
                                              // channel
  int phrasePlayPos_[SONG_CHANNEL_COUNT]; // .Play position in phrase for each
                                          // channel
  int phraseCurPos_;                      // current UI cursor row position

  // Sample Editor
  etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> sampleEditorFilename;
  bool sampleEditorProjectList = false;
};
#endif
