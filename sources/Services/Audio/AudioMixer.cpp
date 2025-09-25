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
#include <algorithm>
#include <cstdint>
#include <string.h> // Required for strcmp

Reverb2 AudioMixer::reverb_;
fixed AudioMixer::renderBuffer_[MAX_SAMPLE_COUNT * 2];

AudioMixer::AudioMixer(const char *name)
    : T_SimpleList<AudioModule>(false), enableRendering_(0), writer_(0),
      name_(name) {
  volume_ = (i2fp(1));
  reverb_wet_ = fl2fp(1.0f);
  reverb_enabled_ = true; // Hardcoded as requested
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

void AudioMixer::EnableReverb(bool enable) { reverb_enabled_ = enable; }

void AudioMixer::SetReverbWet(fixed wet) { reverb_wet_ = wet; }

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
        while (count--) {
          *dst += *src;
          dst++;
          src++;
        }
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
      }
    }
  }

  if (gotData) {
    fixed *c = buffer;
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

  // Always update avgMixerLevel_ regardless of whether we got data
  // This ensures VU meters update properly in all scenarios
  avgMixerLevel_ = fp2i(peakL) << 16;
  avgMixerLevel_ += fp2i(peakR);

  // The reverb is a master effect and should only be
  // applied ONCE by the top-level mixer.
  if (name_ == "Master") {
    if (reverb_enabled_) {
      if (!gotData) {
        // need to clear prev buffer before sendig to reverb
        memset(buffer, 0, samplecount * 2 * sizeof(fixed));
      };
      MasterMixRender(buffer, samplecount);
    }
  }

  // The original rendering block, now only used for writing the final mix to a
  // file.
  if (enableRendering_ && writer_) {
    if (!gotData) {
      memset(buffer, 0, samplecount * 2 * sizeof(fixed));
    };
    writer_->AddBuffer(buffer, samplecount);
  }
  return gotData;
};

void AudioMixer::SetVolume(fixed volume) { volume_ = volume; }

/**
 * @brief Processes a master mix buffer, applying reverb as a send effect.
 *
 * @param master_mix_buffer A stereo interleaved buffer of 16-bit PCM audio.
 * @param buffer_size The number of stereo frames in the buffer (not the total
 * number of samples).
 */
/**
 * @brief Processes a master mix buffer in 32-bit Q16.15 'fixed' format,
 * applying reverb.
 *
 * @param master_mix_buffer A stereo interleaved buffer of 'fixed' (Q16.15)
 * audio.
 * @param buffer_size The number of stereo frames in the buffer.
 */
inline void AudioMixer::MasterMixRender(fixed *master_mix_buffer,
                                        size_t buffer_size) {
  // Define the wet/dry mix level. 0.0 is fully dry, 1.0 is fully wet.
  const float wet_mix_level = 0.5f;
  const q15_t wet_level = f32_to_q15(wet_mix_level);
  const q15_t dry_level = Q15_MAX - wet_level; // Q15_MAX is ~1.0 in Q1.15

  // Buffers to hold the reverb's 16-bit stereo output.
  int16_t reverb_out_l, reverb_out_r;

  size_t stereosample = 0;
  for (size_t i = 0; i < buffer_size; ++i) {
    // 1. Get the current 32-bit 'fixed' dry sample.
    stereosample = i * 2;
    fixed dry_left_32 = master_mix_buffer[stereosample];
    fixed dry_right_32 = master_mix_buffer[stereosample + 1];

    // 2. Convert 32-bit Q16.15 samples to 16-bit Q1.15 for the reverb.
    q15_t dry_left_16 = static_cast<q15_t> fp2i(dry_left_32);
    q15_t dry_right_16 = static_cast<q15_t> fp2i(dry_right_32);

    // 3. Create a mono input for the reverb
    q15_t mono_input = add_q15(dry_left_16 >> 1, dry_right_16 >> 1);

    // 4. Process the 16-bit mono sample through the reverb.
    reverb_.process(mono_input, reverb_out_l, reverb_out_r);

    // 5. Mix the signals. Since the dry signal has been converted to Q1.15,
    // we can perform the entire mix using the safe, saturating q15.h functions.
    // final = (dry * dry_level) + (wet * wet_level)
    q15_t final_left_16 = add_q15(mult_q15(dry_left_16, dry_level),
                                  mult_q15(reverb_out_l, wet_level));
    q15_t final_right_16 = add_q15(mult_q15(dry_right_16, dry_level),
                                   mult_q15(reverb_out_r, wet_level));

    // 6. Convert the final 16-bit Q1.15 result back to 32bit 'fixed' format.
    master_mix_buffer[i * 2] = i2fp(reverb_out_l);
    master_mix_buffer[(i * 2) + 1] = i2fp(reverb_out_r);
  }
}
