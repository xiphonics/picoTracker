#include "AudioFileStreamer.h"
#include "Application/Model/Config.h"
#include "Application/Utils/fixed.h"
#include "System/Console/Trace.h"

AudioFileStreamer::AudioFileStreamer() {
  wav_ = 0;
  mode_ = AFSM_STOPPED;
  newPath_ = false;
};

AudioFileStreamer::~AudioFileStreamer() { SAFE_DELETE(wav_); };

bool AudioFileStreamer::Start(char *name) {
  Trace::Debug("Starting to stream:%s", name);
  strcpy(name_, name);
  newPath_ = true;
  mode_ = AFSM_PLAYING;
  return true;
};

void AudioFileStreamer::Stop() {
  mode_ = AFSM_STOPPED;
  Trace::Debug("Streaming stopped");
};

bool AudioFileStreamer::IsPlaying() { return (mode_ == AFSM_PLAYING); }

bool AudioFileStreamer::Render(fixed *buffer, int samplecount) {

  // See if we're playing
  if (mode_ == AFSM_STOPPED) {
    SAFE_DELETE(wav_);
    return false;
  }

  // Do we need to get a new file ?
  if (newPath_) {
    SAFE_DELETE(wav_);
    newPath_ = false;
  }

  // look if we need to load the file
  if (!wav_) {
    Trace::Log("", "wave open:%s", name_);
    wav_ = WavFile::Open(name_);
    if (!wav_) {
      Trace::Error("Failed to open streaming of file:%s", name_);
      mode_ = AFSM_STOPPED;
      return false;
    }
    position_ = 0;
  }

  // we are playing a valid file

  long size = wav_->GetSize(-1);
  int count = samplecount;
  if (position_ + samplecount >= size) {
    Trace::Debug("Reached the end of %d samples", size);
    count = size - position_;
    mode_ = AFSM_STOPPED;
    memset(buffer, 0, 2 * samplecount * sizeof(fixed));
  }

  fixed *dst = buffer;
  int channel = wav_->GetChannelCount(-1);

  Variable *v = project_->FindVariable(FourCC::VarMasterVolume);
  int vol = v->GetInt();
  fixed volume = fp_mul(i2fp(vol), fl2fp(0.01f));

  while (count > 0) {
    // 64 bytes * 2 bytes resolution * 2 channels = 256
    // which is the half the readBuffer size for files
    // another half of such buffer is needed to expand to
    // 16bits if sample is 8bit
    // TODO: refactor this whole thing to make it less brittle
    long bufferSize = count > 64 ? 64 : count;
    wav_->GetBuffer(position_, bufferSize);
    short *src = (short *)wav_->GetSampleBuffer(-1);

    // I might need to do sample interpolation here
    for (int i = 0; i < bufferSize; i++) {
      fixed v = *dst++ = fp_mul(i2fp(*src++), volume);
      if (channel == 2) {
        // apply master volume when streaming
        *dst++ = fp_mul(i2fp(*src++), volume);
      } else {
        *dst++ = v;
      }
    }
    count -= bufferSize;
    position_ += bufferSize;
  }

  return true;
}