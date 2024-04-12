#include "AudioMixer.h"
#include "Adapters/picoTracker/gui/picoTrackerEventManager.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "System/System/System.h"

fixed AudioMixer::renderBuffer_[MAX_SAMPLE_COUNT * 2];

AudioMixer::AudioMixer(const char *name)
    : T_SimpleList<AudioModule>(false), enableRendering_(0), writer_(0),
      name_(name) {
  volume_ = (i2fp(1));
};

AudioMixer::~AudioMixer() {}

void AudioMixer::SetFileRenderer(const char *path) { renderPath_ = path; };

void AudioMixer::EnableRendering(bool enable) {

  if (enable == enableRendering_) {
    return;
  }

  if (enable) {
    writer_ = new WavFileWriter(renderPath_.c_str());
  }

  enableRendering_ = enable;
  if (!enable) {
    writer_->Close();
    SAFE_DELETE(writer_);
  }
};

bool AudioMixer::Render(fixed *buffer, int samplecount) {
  bool gotData = false;
  for (Begin(); !IsDone(); Next()) {
    AudioModule &current = CurrentItem();

    if (!gotData) {
      gotData = current.Render(buffer, samplecount);
    } else {
      // Hack: there is a series of hierarchical buses that render the output
      // The hierarchy goes 1 AudioMixer - 10 Mixbus - 1 Mixbus - 1
      // PlayerChannel. We only want to parallelize at the point where we have
      // 10 MixBus (each of which is ultimately a PlayerChannel) We only ask
      // core 0 to render some player channels in that case, otherwise we want
      // to handle everything on core1. Since at least the first iteration will
      // run in core1, we know that if we reach this point, it's because we have
      // more than 1, hence we are at the 10 MixBus part of the rendering
      if (!picoTrackerEventManager::current) {
        picoTrackerEventManager::buffer = buffer;
        picoTrackerEventManager::samplecount = samplecount;
        picoTrackerEventManager::current = &current;
        continue;
      }

      if (current.Render(renderBuffer_, samplecount)) {
        // We only have one output buffer, so we use a mutex to write to it
        mutex_enter_blocking(&picoTrackerEventManager::renderMtx);
        fixed *dst = buffer;
        fixed *src = renderBuffer_;
        int count = samplecount * 2;
        while (count--) {
          *dst += *src;
          dst++;
          src++;
        }
        mutex_exit(&picoTrackerEventManager::renderMtx);
        gotData = true;
      }
    }
  }

  // If we finished rendering and core0 is still rendering something, we need
  // to wait until it flushes the buffers
  while ((get_core_num() == 1) and picoTrackerEventManager::current) {
  }
  gotData = gotData ? gotData : picoTrackerEventManager::gotData;

  //  Apply volume
  if (gotData) {
    fixed *c = buffer;
    if (volume_ != i2fp(1)) {
      for (int i = 0; i < samplecount * 2; i++) {
        fixed v = fp_mul(*c, volume_);
        *c++ = v;
      }
    }
  }
  if (enableRendering_ && writer_) {
    if (!gotData) {
      memset(buffer, 0, samplecount * 2 * sizeof(fixed));
    };
    writer_->AddBuffer(buffer, samplecount);
  }

  return gotData;
};

void AudioMixer::SetVolume(fixed volume) { volume_ = volume; }
