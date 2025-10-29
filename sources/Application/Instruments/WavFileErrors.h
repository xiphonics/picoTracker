/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _WAV_FILE_ERRORS_H_
#define _WAV_FILE_ERRORS_H_

enum WAVEFILE_ERROR {
  INVALID_FILE,
  UNSUPPORTED_FILE_FORMAT,
  INVALID_HEADER,
  UNSUPPORTED_WAV_FORMAT,
  UNSUPPORTED_COMPRESSION,
  UNSUPPORTED_BITDEPTH,
  UNSUPPORTED_SAMPLERATE,
};

#endif
