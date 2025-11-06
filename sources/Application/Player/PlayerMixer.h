/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _APPLICATION_MIXER_H_
#define _APPLICATION_MIXER_H_

#include "Application/Audio/AudioFileStreamer.h"
#include "Application/Audio/RecordStreamer.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Model/Project.h"
#include "Application/Utils/fixed.h"
#include "Application/Views/ViewData.h"
#include "Externals/etl/include/etl/singleton.h"
#include "Foundation/Observable.h"
#include "PlayerChannel.h"
#include "Services/Audio/AudioOut.h"

#define STREAM_MIX_BUS 8

class PlayerMixerBase : public Observable, public I_Observer {
public:
  bool Start();
  void Stop();
  bool IsPlaying();
  bool Init(Project *project);
  void Close();

  void OnPlayerStart(MixerServiceMode msmMode);
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

  void StartStreaming(const char *name, int startSample = 0);
  void StartLoopingStreaming(const char *name);
  void StopStreaming();

  void StartRecordStreaming(uint16_t *srcBuffer, uint32_t size, bool stereo);
  void StopRecordStreaming();

  stereosample GetMasterOutLevel();

  void Update(Observable &o, I_ObservableData *d);
  int GetPlayedBufferPercentage();

  void SetChannelMute(int channel, bool mute);
  bool IsChannelMuted(int channel);

  const char *GetPlayedNote(int channel);
  const char *GetPlayedOctive(int channel);

  AudioOut *GetAudioOut();

  void Lock();
  void Unlock();

  // Get the current project
  Project *GetProject() { return project_; }

  etl::array<stereosample, SONG_CHANNEL_COUNT> *GetMixerLevels();

private:
  // Only allow etl::singleton to construct
  friend class etl::singleton<PlayerMixerBase>;
  PlayerMixerBase();

  Project *project_;
  etl::array<stereosample, SONG_CHANNEL_COUNT> mixerLevels_;

  I_Instrument *lastInstrument_[SONG_CHANNEL_COUNT];
  bool isChannelPlaying_[SONG_CHANNEL_COUNT];

  AudioFileStreamer fileStreamer_;
  RecordStreamer recordStreamer_;
  PlayerChannel *channel_[SONG_CHANNEL_COUNT];

  // store trigger notes, 0xFF = none

  unsigned char notes_[SONG_CHANNEL_COUNT];
};

using PlayerMixerSingleton = etl::singleton<PlayerMixerBase>;
inline PlayerMixerBase &PlayerMixer() {
  return PlayerMixerSingleton::instance();
}
#endif
