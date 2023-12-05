
#ifndef _MIXER_H_
#define _MIXER_H_

#include "Application/Persistency/Persistent.h"
#include "Foundation/T_Singleton.h"

#include "Application/Utils/fixed.h"
#include "Song.h"

class Mixer : public T_Singleton<Mixer>, Persistent {
public:
  Mixer();
  ~Mixer();
  void Clear();

  inline int GetBus(int i) { return channelBus_[i]; };

  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

private:
  char channelBus_[SONG_CHANNEL_COUNT];
};

#endif
