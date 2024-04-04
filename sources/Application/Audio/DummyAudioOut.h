#ifndef _DUMMY_AUDIO_OUT_
#define _DUMMY_AUDIO_OUT_

#include "Services/Audio/AudioOut.h"
#include "System/Process/Process.h"

class DummyAudioOut;

class DummyOutThread : public SysThread {
public:
  DummyOutThread(DummyAudioOut *out);
  virtual bool Execute();

private:
  DummyAudioOut *out_;
};

class DummyAudioOut : public AudioOut {
public:
  DummyAudioOut();
  virtual ~DummyAudioOut();

  virtual bool Init();
  virtual void Close();
  virtual bool Start();
  virtual void Stop();

  virtual void Trigger();

  virtual bool Clipped() { return false; };

  virtual int GetPlayedBufferPercentage() { return 0; };

  virtual short GetLastPeakL() { return 0; };
  virtual short GetLastPeakR() { return 0; };

  void SendPulse();

  virtual std::string GetAudioAPI();
  virtual std::string GetAudioDevice();
  virtual int GetAudioBufferSize();
  virtual int GetAudioRequestedBufferSize();
  virtual int GetAudioPreBufferCount();
  virtual double GetStreamTime();

private:
  DummyOutThread *thread_;
  fixed *primarySoundBuffer_;
};

#endif
