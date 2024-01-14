
#include "AudioDriver.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"
#include "System/System/System.h"

AudioBufferData AudioDriver::pool_[SOUND_BUFFER_COUNT];

AudioDriver::AudioDriver(AudioSettings &settings) { settings_ = settings; }

AudioDriver::~AudioDriver() {}

bool AudioDriver::Init() {

  // Clear all buffers

  for (int i = 0; i < SOUND_BUFFER_COUNT; i++) {
    pool_[i].size_ = 0;
    pool_[i].empty_ = true;
  };
  isPlaying_ = false;

  return InitDriver();
}

void AudioDriver::Close() { CloseDriver(); };

bool AudioDriver::Start() {

  isPlaying_ = true;

  poolQueuePosition_ = 0;
  poolPlayPosition_ = 0;
  hasData_ = false;

  return StartDriver();
};

void AudioDriver::Stop() {
  isPlaying_ = false;
  hasData_ = false;
  StopDriver();
}

void AudioDriver::AddBuffer(short *buffer, int samplecount) {
  int len = samplecount * 2 * sizeof(short);

  if (!isPlaying_)
    return;

  if (len > SOUND_BUFFER_MAX) {
    Trace::Error("Alert: buffer size exceeded");
  }

  if (!pool_[poolQueuePosition_].empty_) {
    NInvalid;
    Trace::Error("Audio overrun, please report");
    pool_[poolQueuePosition_].empty_ = true;
    return;
  }

  SYS_MEMCPY(pool_[poolQueuePosition_].buffer_, (char *)buffer, len);
  pool_[poolQueuePosition_].size_ = len;
  pool_[poolQueuePosition_].empty_ = false;
  poolQueuePosition_ = (poolQueuePosition_ + 1) % SOUND_BUFFER_COUNT;
  hasData_ = true;
}

void AudioDriver::OnNewBufferNeeded() {
  SetChanged();
  Event event(Event::ADET_BUFFERNEEDED);
  NotifyObservers(&event);
};

void AudioDriver::onAudioBufferTick() {
  SetChanged();
  Event event(Event::ADET_DRIVERTICK);
  NotifyObservers(&event);
}

bool AudioDriver::hasData() { return hasData_; };

AudioSettings AudioDriver::GetAudioSettings() { return settings_; };
