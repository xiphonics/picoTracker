/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _MIXER_SERVICE_H_
#define _MIXER_SERVICE_H_

#include "Application/Commands/ApplicationCommandDispatcher.h" // Would be better done externally and call an API here
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "MixBus.h"
#include "Services/Audio/AudioMixer.h"
#include "Services/Audio/AudioOut.h"
#include "System/Process/SysMutex.h"

enum MixerServiceMode {
  MSM_AUDIO,
  MSM_FILE,
  MSM_FILESPLIT,
};

#define MAX_BUS_COUNT 10

class MixerService : public T_Singleton<MixerService>,
                     public Observable,
                     public I_Observer,
                     public CommandExecuter {

public:
  MixerService();
  virtual ~MixerService();

  bool Init();
  void Close();

  bool Start();
  void Stop();

  MixBus *GetMixBus(int i);

  MixBus *GetMasterBus() { return &master_; };

  virtual void Update(Observable &o, I_ObservableData *d);

  void OnPlayerStart(MixerServiceMode mode);
  void OnPlayerStop();

  void UpdateMixVolumes();
  int GetPlayedBufferPercentage();

  virtual void Execute(FourCC id, float value);

  AudioOut *GetAudioOut();

  void Lock();
  void Unlock();

protected:
  void setRenderingMode(MixerServiceMode mode);

  // Helper function to convert linear volume (0-100) to non-linear (0.0-1.0) in
  // fixed point
  fixed ToLogVolume(int vol);

private:
  bool configureRenderPaths();

  AudioOut *out_;
  MixBus master_;
  MixBus bus_[MAX_BUS_COUNT];
  SysMutex *sync_;
  Project *project_; // Reference to the current project
};
#endif
