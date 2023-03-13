#ifndef _SONG_H_
#define _SONG_H_

#include "Chain.h"
#include "Phrase.h"
#include "Application/Persistency/Persistent.h"

#define SONG_CHANNEL_COUNT 8
#define SONG_ROW_COUNT 256

#ifndef PICOBUILD
#define MAX_SAMPLEINSTRUMENT_COUNT 0x80
#define MAX_MIDIINSTRUMENT_COUNT 0x10
#else
#define MAX_SAMPLEINSTRUMENT_COUNT 0x0D
#define MAX_MIDIINSTRUMENT_COUNT 0x00
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
