/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef __BASIC_TYPEDEFS__
#define __BASIC_TYPEDEFS__

/* ----------------------------------------------------------------------------
 */

// basic types:
/* COMMENTED FOR PSP

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;

*/

/* ----------------------------------------------------------------------------
 */

short Swap16(short from);
int Swap32(int from);

#endif // __BASIC_TYPEDEFS__
