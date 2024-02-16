#ifndef _SONG_H_
#define _SONG_H_

#include "Chain.h"
#include "Phrase.h"
#include "Application/Persistency/Persistent.h"

#ifndef PICOBUILD
#define SONG_CHANNEL_COUNT 8
#define SONG_ROW_COUNT 256

#define MAX_SAMPLEINSTRUMENT_COUNT 0x80
#define MAX_MIDIINSTRUMENT_COUNT 0x10
#else
#define SONG_CHANNEL_COUNT 8
#define SONG_ROW_COUNT 128

#define MAX_SAMPLEINSTRUMENT_COUNT 0x01
#define MAX_MIDIINSTRUMENT_COUNT 0x10
#endif

#define MAX_INSTRUMENT_COUNT (MAX_SAMPLEINSTRUMENT_COUNT+MAX_MIDIINSTRUMENT_COUNT)

class Song:Persistent {
public:
	Song() ;
	~Song() ;

  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

  unsigned char *data_;
	Chain *chain_ ;
	Phrase *phrase_ ;
} ;

#endif
