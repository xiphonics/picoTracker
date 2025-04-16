#include "AudioFileStreamer.h"
#include "Application/Model/Config.h"
#include "Application/Utils/fixed.h"
#include "Services/Audio/Audio.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "System/io/Status.h"
#include <string.h>

// Initialize the static buffer for single cycle waveforms
short AudioFileStreamer::singleCycleBuffer_[SINGLE_CYCLE_MAX_SAMPLE_SIZE] = {0};

AudioFileStreamer::AudioFileStreamer() {
  wav_ = 0;
  mode_ = AFSM_STOPPED;
  newPath_ = false;
  position_ = 0;
  fileSampleRate_ = 44100;   // Default
  systemSampleRate_ = 44100; // Default
  fpSpeed_ = FP_ONE;         // Default 1.0 in fixed point
  isSingleCycle_ = false;
  cycleLength_ = 0;
  project_ = NULL;
  singleCycleData_ = NULL;
};

AudioFileStreamer::~AudioFileStreamer() { SAFE_DELETE(wav_); };

bool AudioFileStreamer::Start(char *name) {
  Trace::Debug("Starting to stream:%s", name);
  strcpy(name_, name);
  newPath_ = true;
  mode_ = AFSM_PLAYING;
  isSingleCycle_ = false;
  return true;
};

bool AudioFileStreamer::StartLooping(char *name) {
  Trace::Debug("Starting to loop:%s", name);
  strcpy(name_, name);
  newPath_ = true;
  mode_ = AFSM_LOOPING;
  isSingleCycle_ = true;
  return true;
};

void AudioFileStreamer::Stop() {
  mode_ = AFSM_STOPPED;
  Trace::Debug("Streaming stopped");
};

bool AudioFileStreamer::IsPlaying() {
  return (mode_ == AFSM_PLAYING || mode_ == AFSM_LOOPING);
}

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
    int channels = wav_->GetChannelCount(-1);
    long size = wav_->GetSize(-1);
    cycleLength_ = size;

    // Calculate the speed factor for sample rate conversion
    float ratio;

    if (mode_ == AFSM_LOOPING) {
      // For waveforms in looping mode, adjust playback speed to play at C3
      // (130.81 Hz) C3 frequency = 130.81 Hz To get this frequency, we need to
      // play the entire cycle 130.81 times per second So the effective sample
      // rate = cycle length * 130.81
      float effectiveSampleRate = size * 130.81f;
      ratio = effectiveSampleRate / (float)systemSampleRate_;
      Trace::Debug("Single cycle playback at C3: cycle length=%ld, effective "
                   "sample rate=%.2f Hz",
                   size, effectiveSampleRate);
    } else {
      // Normal playback uses the file's sample rate
      ratio = (float)fileSampleRate_ / (float)systemSampleRate_;
    }

    fpSpeed_ = fl2fp(ratio);
    Trace::Debug("AudioFileStreamer: File '%s' - Sample Rate: %d Hz, Channels: "
                 "%d, Size: %ld samples, Speed: %d",
                 name_, fileSampleRate_, channels, size, fp2i(fpSpeed_));

    // Load the entire buffer for single cycle waveforms
    if (mode_ == AFSM_LOOPING &&
        size < 1024) { // 1024 samples = 2048 bytes for stereo
      isSingleCycle_ = true;

      // Get channel count before using it
      int channels = wav_->GetChannelCount(-1);

      // For safety, load the waveform in smaller chunks to avoid buffer
      // overflow FLASH_PAGE_SIZE  and the assertion in  WavFile::GetBuffer
      // requires size < FLASH_PAGE_SIZE/2, so we use 64 as a safe chunk size
      const int SAFE_CHUNK_SIZE = 64; // in samples
      int remainingSize = size;
      int currentPos = 0;

      while (remainingSize > 0) {
        // Calculate the chunk size for this iteration
        int chunkSize =
            (remainingSize > SAFE_CHUNK_SIZE) ? SAFE_CHUNK_SIZE : remainingSize;

        // Load this chunk
        if (!wav_->GetBuffer(currentPos, chunkSize)) {
          Trace::Error("Failed to load chunk at position %d, size %d",
                       currentPos, chunkSize);
          break;
        }

        // Copy the data to our static buffer
        short *srcBuffer = (short *)wav_->GetSampleBuffer(-1);
        if (srcBuffer) {
          // Calculate destination position in our buffer
          short *destPos = singleCycleBuffer_ + (currentPos * channels);

          // Copy this chunk to our static buffer
          int bytesToCopy = chunkSize * channels * sizeof(short);
          memcpy(destPos, srcBuffer, bytesToCopy);

          Trace::Debug("Loaded chunk of single cycle waveform: pos=%d, size=%d "
                       "(%d bytes)",
                       currentPos, chunkSize, bytesToCopy);
        } else {
          Trace::Error("Failed to get sample buffer for chunk at position %d",
                       currentPos);
          break;
        }

        // Move to the next chunk
        currentPos += chunkSize;
        remainingSize -= chunkSize;
      }

      // Set the pointer to our buffer
      if (remainingSize == 0) {
        singleCycleData_ = singleCycleBuffer_;
        Trace::Debug(
            "Successfully loaded entire single cycle waveform: %ld samples",
            size);
      } else {
        // We didn't load the entire waveform, so don't use it
        isSingleCycle_ = false;
        Trace::Error("Failed to load entire single cycle waveform");
      }
    }
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

  // Special handling for single cycle waveforms in looping mode
  if (mode_ == AFSM_LOOPING && isSingleCycle_) {
    // Use our static buffer that we've already loaded with the entire waveform
    if (!singleCycleData_) {
      Trace::Error("AudioFileStreamer: Single cycle buffer is null");
      mode_ = AFSM_STOPPED;
      return false;
    }

    // Use the static buffer as our source
    short *src = singleCycleData_;

    // Process each output sample
    float pos = position_;
    float cycleSize = (float)size;

    for (int i = 0; i < samplecount; i++) {
      // Calculate the source position, ensuring we wrap within the cycle
      int sourcePos = (int)pos;
      if (sourcePos >= size) {
        sourcePos = sourcePos % size;
      }

      // Get the current sample
      short *currentSample = src + (sourcePos * channelCount);

      // Use the exact sample value without interpolation
      if (channelCount == 2) {
        // Stereo sample
        fixed leftSample = i2fp(currentSample[0]);
        fixed rightSample = i2fp(currentSample[1]);

        // Apply volume
        *dst++ = fp_mul(leftSample, volume);
        *dst++ = fp_mul(rightSample, volume);
      } else {
        // Mono sample
        fixed sample = i2fp(currentSample[0]);

        // Apply volume and duplicate to both channels
        fixed v = fp_mul(sample, volume);
        *dst++ = v;
        *dst++ = v;
      }

      // Move to the next position and wrap around if needed
      pos += fp2fl(fpSpeed_);
      while (pos >= cycleSize) {
        pos -= cycleSize;
      }
    }

    // Update the position for the next render call
    position_ = pos;
    return true;
  }

  // Standard playback for normal samples
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