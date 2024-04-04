#ifndef _MIXER_SERVICE_H_
#define _MIXER_SERVICE_H_

#include "Application/Commands/CommandDispatcher.h" // Would be better done externally and call an API here
#include "Foundation/Observable.h"
#include "Foundation/T_Singleton.h"
#include "MixBus.h"
#include "Services/Audio/AudioMixer.h"
#include "Services/Audio/AudioOut.h"
#ifndef PICOBUILD
#include "SDL/SDL.h"
#else
#include "pico/mutex.h"
#endif

enum MixerServiceMode {
  MSM_AUDIO,
  MSM_FILE,
  MSM_FILESPLIT,
  MSM_FILERT,
  MSM_FILESPLITRT
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

  virtual void Update(Observable &o, I_ObservableData *d);

  void OnPlayerStart();
  void OnPlayerStop();

  bool Clipped();
  void SetMasterVolume(int);
  int GetPlayedBufferPercentage();

  int GetAudioPeakL();
  int GetAudioPeakR();

  virtual void Execute(FourCC id, float value);

  AudioOut *GetAudioOut();

  void Lock();
  void Unlock();

protected:
  void toggleRendering(bool enable);

private:
  AudioOut *out_;
  MixBus master_;
  MixBus bus_[MAX_BUS_COUNT];
  MixerServiceMode mode_;
#ifndef PICOBUILD
  SDL_mutex *sync_;
#else
  mutex_t *sync_;
#endif
};
#endif
