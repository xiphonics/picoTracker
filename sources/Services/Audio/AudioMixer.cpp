#include "AudioMixer.h"
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

stereosample AudioMixer::Render(fixed *buffer, int samplecount) {

  // FP_ZERO represents also that no data was rendered, ie. zero volume for the
  // whole buffer
  stereosample avgLevels = 0;

  u_int8_t index = 0;
  for (Begin(); !IsDone(); Next()) {
    AudioModule &current = CurrentItem();
    if (avgLevels == 0) {
      avgLevels = current.Render(buffer, samplecount);
      avgModuleLevels_[index] = avgLevels;
    } else {
      if (current.Render(renderBuffer_, samplecount)) {
        memcpy(buffer, renderBuffer_, samplecount * 2 * sizeof(fixed));
      }
    }
  }

  //  Apply volume
  if (avgLevels != FP_ZERO) {
    fixed *c = buffer;
    if (volume_ != i2fp(1)) {
      for (int i = 0; i < samplecount * 2; i++) {
        fixed v = fp_mul(*c, volume_);
        *c++ = v;
      }
    }
  }
  if (enableRendering_ && writer_) {
    if (avgLevels == FP_ZERO) {
      memset(buffer, 0, samplecount * 2 * sizeof(fixed));
    };
    writer_->AddBuffer(buffer, samplecount);
  }
  return avgLevels; // TODO: return actual levels?
};

void AudioMixer::SetVolume(fixed volume) { volume_ = volume; }
