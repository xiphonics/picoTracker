/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "SampleEditProgressDisplay.h"

#include "System/io/Status.h"

SampleEditProgressDisplay::SampleEditProgressDisplay(
    const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> &filename)
    : sampleName_(filename), spinnerIndex_(0) {}

void SampleEditProgressDisplay::Update(uint8_t percent) {
  const char spinner = spinnerChars_[spinnerIndex_++ & 0x03];
  Status::Set("Sample edit\n%s\n%3u%% %c", sampleName_.c_str(),
                       static_cast<unsigned int>(percent), spinner);
}

void SampleEditProgressDisplay::Finish(bool success) {
  const char *result = success ? "Complete" : "Failed";
  Status::Set("Sample edit %s\n%s", result, sampleName_.c_str());
}
