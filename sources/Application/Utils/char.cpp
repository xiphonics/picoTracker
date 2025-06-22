/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "char.h"

char h2c__[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

const char *noteNames[12] = {"C ", "C#", "D ", "D#", "E ", "F ",
                             "F#", "G ", "G#", "A ", "A#", "B "};
