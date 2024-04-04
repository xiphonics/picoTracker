
#ifndef _APPLICATION_MIXER_H_
#define _APPLICATION_MIXER_H_

#include "Application/Audio/AudioFileStreamer.h"
#include "Application/Model/Project.h"
#include "Application/Utils/fixed.h"
#include "Application/Views/ViewData.h"
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "PlayerChannel.h"
#include "Services/Audio/AudioOut.h"

#define STREAM_MIX_BUS 8

class PlayerMixer : public T_Singleton<PlayerMixer>,
                    public Observable,
                    public I_Observer {
public:
  PlayerMixer();
  virtual ~PlayerMixer(){};

  bool Start();
  void Stop();
  bool IsPlaying();
  bool Init(Project *project);
  void Close();

  void OnPlayerStart();
  void OnPlayerStop();

  void StartInstrument(int channel, I_Instrument *instrument,
                       unsigned char note, bool newInstrument);
  void StopInstrument(int channel);

  int GetChannelNote(int Channel);

  I_Instrument *GetInstrument(int channel);

  I_Instrument *GetLastInstrument(int channel);

  void StartChannel(int channel);
  void StopChannel(int channel);

  bool IsChannelPlaying(int channel);

  void StartStreaming(const Path &);
  void StopStreaming();

  bool Clipped();

  short GetAudioLevelL();
  short GetAudioLevelR();

  void Update(Observable &o, I_ObservableData *d);
  int GetPlayedBufferPercentage();

  void SetChannelMute(int channel, bool mute);
  bool IsChannelMuted(int channel);

  const char *GetPlayedNote(int channel);
  const char *GetPlayedOctive(int channel);

  AudioOut *GetAudioOut();

  void Lock();
  void Unlock();

private:
  Project *project_;
  bool clipped_;
  short peakL_;
  short peakR_;

  I_Instrument *lastInstrument_[SONG_CHANNEL_COUNT];
  bool isChannelPlaying_[SONG_CHANNEL_COUNT];

  AudioFileStreamer fileStreamer_;
  PlayerChannel *channel_[SONG_CHANNEL_COUNT];

  // store trigger notes, 0xFF = none

  unsigned char notes_[SONG_CHANNEL_COUNT];
};

#endif
