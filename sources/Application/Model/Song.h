#ifndef _SONG_H_
#define _SONG_H_

#include "Application/Persistency/Persistent.h"
#include "Chain.h"
#include "Phrase.h"

#define SONG_CHANNEL_COUNT 8
#define SONG_ROW_COUNT 128

#define MAX_SAMPLEINSTRUMENT_COUNT 0x10
#define MAX_MIDIINSTRUMENT_COUNT 0x10
#define MAX_TINYSYNTH_INSTRUMENT_COUNT 0x01

#define MAX_INSTRUMENT_COUNT                                                   \
  (MAX_SAMPLEINSTRUMENT_COUNT + MAX_MIDIINSTRUMENT_COUNT +                     \
   MAX_TINYSYNTH_INSTRUMENT_COUNT)

class Song : Persistent {
public:
  Song();
  ~Song();

  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

  unsigned char data_[SONG_CHANNEL_COUNT * SONG_ROW_COUNT];
  Chain chain_;
  Phrase phrase_;
};

#endif
