/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "AudioMixer.h"
#include "System/Console/Trace.h"
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
  fixed peakL = 0;
  fixed peakR = 0;

  for (Begin(); !IsDone(); Next()) {
    AudioModule &current = CurrentItem();
    if (!gotData) {
      gotData = current.Render(buffer, samplecount);
    } else {
      if (current.Render(renderBuffer_, samplecount)) {
        fixed *dst = buffer;
        fixed *src = renderBuffer_;
        int count = samplecount * 2;

        /* Manually unrolling the loop gives a 25% performance increase (from
         * 6500 cycles for 800 samples to 5100 cycles. This is due to being able
         * to schedule independent loads simultanously, reduce load latency by
         * loading independent loads instead of immediately executing a
         * dependent action (add) having to wait for the load to complete.
         * Potentially also reducing the load comparison overhead 16
         * instructions in the unroll gives the best performance, 8 gave ~17%
         * improvement.
         */
        int i = 0;
        for (; i + 16 <= count; i += 16) {
          dst[i + 0] += src[i + 0];
          dst[i + 1] += src[i + 1];
          dst[i + 2] += src[i + 2];
          dst[i + 3] += src[i + 3];
          dst[i + 4] += src[i + 4];
          dst[i + 5] += src[i + 5];
          dst[i + 6] += src[i + 6];
          dst[i + 7] += src[i + 7];
          dst[i + 8] += src[i + 8];
          dst[i + 9] += src[i + 9];
          dst[i + 10] += src[i + 10];
          dst[i + 11] += src[i + 11];
          dst[i + 12] += src[i + 12];
          dst[i + 13] += src[i + 13];
          dst[i + 14] += src[i + 14];
          dst[i + 15] += src[i + 15];
        }
        for (; i < count; ++i)
          dst[i] += src[i];
      }
    }
  }

  //  Apply volume to mix of all of this instances "sub" audiomixers
  // TODO (democloid): This is wildly inefficient, doing this loop takes 4 - 5
  // times the time it takes a mix loop above. Some tests show that at least
  // double performance is not hard to achieve
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
  }

  // Always update avgMixerLevel_ regardless of whether we got data
  // This ensures VU meters update properly in all scenarios
  avgMixerLevel_ = fp2i(peakL) << 16;
  avgMixerLevel_ += fp2i(peakR);

  if (enableRendering_ && writer_) {
    if (!gotData) {
      memset(buffer, 0, samplecount * 2 * sizeof(fixed));
    };
    writer_->AddBuffer(buffer, samplecount);
  }
  return gotData;
};

void AudioMixer::SetVolume(fixed volume) { volume_ = volume; }
