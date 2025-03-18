#include "AudioMixer.h"
#include "System/System/System.h"

fixed AudioMixer::renderBuffer_[MAX_SAMPLE_COUNT * 2];

AudioMixer::AudioMixer(const char *name)
    : T_SimpleList<AudioModule>(false), enableRendering_(0), writer_(0),
      name_(name) {
  volume_ = (i2fp(1));
  Trace::Debug("AudioMixer [%s]", name);
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
  fixed peakL = 0;
  fixed peakR = 0;

  for (Begin(); !IsDone(); Next()) {
    AudioModule &current = CurrentItem();
    if (avgLevel == FP_ZERO) {
      avgLevel = current.Render(buffer, samplecount);
      avgModuleLevels_[index] = avgLevel;
    } else {
      if (current.Render(renderBuffer_, samplecount)) {
        memcpy(buffer, renderBuffer_, samplecount * 2 * sizeof(fixed));
      }
    }
  }

  //  Apply volume to mix of all of this instances "sub" audiomixers
  if (gotData) {
    fixed *c = buffer;
    if (volume_ != i2fp(1)) {
      for (int i = 0; i < samplecount * 2; i++) {
        fixed v = fp_mul(*c, volume_);
        *c++ = v;

        if (i % 2 == 0 && v >= peakR) {
          peakR = v;
        }
        if (v >= peakL) {
          peakL = v;
        }
      }
    } else {
      for (int i = 0; i < samplecount * 2; i++) {
        fixed v = buffer[i];
        if (i % 2 == 0 && v >= peakR) {
          peakR = v;
        }
        if (v >= peakL) {
          peakL = v;
        }
      }
    }
    avgMixerLevel_ = fp2i(peakL) << 16;
    avgMixerLevel_ += fp2i(peakR);
  }

  if (enableRendering_ && writer_) {
    if (avgLevel == FP_ZERO) {
      memset(buffer, 0, samplecount * 2 * sizeof(fixed));
    };
    writer_->AddBuffer(buffer, samplecount);
  }
  return avgLevel;
};

void AudioMixer::SetVolume(fixed volume) { volume_ = volume; }
