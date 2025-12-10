/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef SAMPLE_EDIT_PROGRESS_DISPLAY_H_
#define SAMPLE_EDIT_PROGRESS_DISPLAY_H_

#include "Application/Persistency/PersistenceConstants.h"
#include "Externals/etl/include/etl/string.h"

class SampleEditProgressDisplay {
public:
  explicit SampleEditProgressDisplay(
      const etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> &filename);

  void Update(uint8_t percent);
  void Finish(bool success);

private:
  static constexpr char spinnerChars_[4] = {'|', '/', '-', '\\'};
  etl::string<MAX_INSTRUMENT_FILENAME_LENGTH> sampleName_;
  uint8_t spinnerIndex_;
};

#endif // SAMPLE_EDIT_PROGRESS_DISPLAY_H_
