#include "AudioFileStreamer.h"
#include "Application/Model/Config.h"
#include "Application/Utils/fixed.h"
#include "Services/Audio/Audio.h"
#include "System/Console/Trace.h"

AudioFileStreamer::AudioFileStreamer() {
  wav_ = 0;
  mode_ = AFSM_STOPPED;
  newPath_ = false;
  position_ = 0;
  fileSampleRate_ = 44100;   // Default
  systemSampleRate_ = 44100; // Default
  fpSpeed_ = FP_ONE;         // Default 1.0 in fixed point
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

    // Get sample rate information and calculate speed factor
    fileSampleRate_ = wav_->GetSampleRate(-1);
    systemSampleRate_ = Audio::GetInstance()->GetSampleRate();

    // Calculate the speed factor for sample rate conversion
    float ratio = (float)fileSampleRate_ / (float)systemSampleRate_;
    fpSpeed_ = fl2fp(ratio);

    int channels = wav_->GetChannelCount(-1);
    long size = wav_->GetSize(-1);
    Trace::Debug("AudioFileStreamer: File '%s' - Sample Rate: %d Hz, Channels: "
                 "%d, Size: %ld samples, Speed: %d",
                 name_, fileSampleRate_, channels, size, fp2i(fpSpeed_));
  }

  // We are playing a valid file
  long size = wav_->GetSize(-1);
  int channelCount = wav_->GetChannelCount(-1);

  // Clear the output buffer
  SYS_MEMSET(buffer, 0, samplecount * 2 * sizeof(fixed));

  // Get volume from project
  Variable *v = project_->FindVariable(FourCC::VarMasterVolume);
  int vol = v->GetInt();
  fixed volume = fp_mul(i2fp(vol), fl2fp(0.01f));

  fixed *dst = buffer;

  // Read a small buffer and resample directly
  int bufferSize = 64; // Small, safe buffer size

  // Check if we're near the end of the file
  if (position_ + (samplecount * fp2fl(fpSpeed_)) >= size) {
    // We'll reach the end during this render call
    int remainingSamples = size - position_;
    if (remainingSamples <= 0) {
      mode_ = AFSM_STOPPED;
      return false;
    }
  }

  // Read the samples from the file - just once at the current position
  if (!wav_->GetBuffer((int)position_, bufferSize)) {
    Trace::Error("AudioFileStreamer: Failed to get buffer at position %d",
                 (int)position_);
    mode_ = AFSM_STOPPED;
    return false;
  }

  // Get the sample buffer
  short *src = (short *)wav_->GetSampleBuffer(-1);
  if (!src) {
    Trace::Error("AudioFileStreamer: GetSampleBuffer returned null");
    mode_ = AFSM_STOPPED;
    return false;
  }

  // Process each output sample
  float pos = position_;

  for (int i = 0; i < samplecount; i++) {
    // Check if we need to read more samples
    if ((int)pos >= (int)position_ + bufferSize - 1) {
      // We've moved past our buffer, need to read more
      position_ = (int)pos;

      // Check if we've reached the end of the file
      if (position_ >= size - 1) {
        mode_ = AFSM_STOPPED;
        return false;
      }

      // Read the next buffer
      if (!wav_->GetBuffer((int)position_, bufferSize)) {
        Trace::Error("AudioFileStreamer: Failed to get buffer at position %d",
                     (int)position_);
        mode_ = AFSM_STOPPED;
        return false;
      }

      // Update the source pointer
      src = (short *)wav_->GetSampleBuffer(-1);
      if (!src) {
        Trace::Error("AudioFileStreamer: GetSampleBuffer returned null");
        mode_ = AFSM_STOPPED;
        return false;
      }

      // Adjust position to be relative to the new buffer
      pos = position_;
    }

    // Calculate the source position and fractional part
    int sourcePos = (int)(pos - position_);
    float frac = pos - (position_ + sourcePos);

    // Get the current and next sample for interpolation
    short *currentSample = src + (sourcePos * channelCount);
    short *nextSample = currentSample + channelCount;

    // Linear interpolation between samples
    fixed fpFrac = fl2fp(frac);
    fixed s1 = i2fp(currentSample[0]);
    fixed s2 = i2fp(nextSample[0]);
    fixed leftSample = s1 + fp_mul(fpFrac, s2 - s1);
    fixed rightSample;

    if (channelCount == 2) {
      fixed s3 = i2fp(currentSample[1]);
      fixed s4 = i2fp(nextSample[1]);
      rightSample = s3 + fp_mul(fpFrac, s4 - s3);
    } else {
      rightSample = leftSample;
    }
    // Apply volume
    *dst++ = fp_mul(leftSample, volume);
    *dst++ = fp_mul(rightSample, volume);

    // Move to the next source position based on the sample rate ratio
    pos += fp2fl(fpSpeed_);
  }

  // Update the position for the next render call
  position_ = pos;

  // If we've reached the end of the file, stop playback
  if (position_ >= size) {
    mode_ = AFSM_STOPPED;
  }

  return true;
}