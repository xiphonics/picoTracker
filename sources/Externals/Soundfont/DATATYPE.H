     /*********************************************************************
     
     datatype.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
/******************************************************************************
*
*     Copyright (c) E-mu Systems, Inc. 1994. All rights Reserved.
*
*******************************************************************************
*/

/*****************************************************************************
* @(#)datatype.h        1.1 12:06:28 3/15/95 12:06:35
*
* Filename: datatype.h
*
* Description: Header file for SoundFont enabler Data Types
*
*******************************************************************************
*/

#ifndef __DATATYPE_H
#define __DATATYPE_H

/************
* Includes
************/

#include <limits.h>


/************
* Defines
************/

#ifndef __BYTE_INCOHERENT    /* Big    Endian  (IE 680x0) */
#define __BYTE_COHERENT      /* Little Endian  (IE 80x86) */
#endif

#ifdef _UNIX_STUB_
#define pascal
#define signed
#endif

#ifdef EMU_WINDOWS
  #include <windows.h>
#else
  #define _export
#endif

/* Expected data values */
#define CHAR_MINVAL  -127
#define CHAR_MAXVAL   127
#define BYTE_MAXVAL   255
#define SHRT_MINVAL  -32767
#define SHRT_MAXVAL   32767
#define LONG_MINVAL  -2147483647L
#define LONG_MAXVAL   2147483647L
#define DWORD_MAXVAL  4294967295L

#ifndef FALSE
  #define FALSE  0
#endif

#ifndef TRUE
  #define TRUE   1
#endif

#ifndef EMU_WINDOWS // windows.h defines these already
//  #define LOBYTE(x)  ((x) & 0x00FF)
//  #define HIBYTE(x)  (((x) & 0xFF00) >> 8)
#endif


/*************
* Typedefs
*************/

typedef char                CHAR;      /*  8 bit signed value          */
typedef short               SHORT;     /* 16 bit signed value was: INT */

#ifdef EMU_WINDOWS
  /*****************************************************************
  * These idiosyncratic pointer definitions for memory allocations
  * which are greater than 64K and are Intel-centric compiling
  * environment necessities, at least for Windows environments.
  * Perhaps, however, Windows NT will eliminate the 'huge' keyword
  * requirement altogether.
  *****************************************************************/
  typedef BYTE  huge* BYTEPTR;
  typedef WORD  huge* UINTPTR;
  typedef DWORD huge* DWORDPTR;
  typedef void  huge* VOIDPTR;
#else
  typedef unsigned char       BYTE;    /*  8 bit unsigned value   */
#ifndef BOOL
  typedef int               BOOL;    /* 16 bit signed value     */
#endif
  typedef unsigned short      WORD;    /* 16 bit signed value     */
  typedef signed long         LONG;    /* 32 bit signed value     */
  typedef unsigned long       DWORD;   /* 32 bit unsigned value   */
  typedef float               FLOAT;   /* 32 bit floating point value */
  typedef double              DOUBLE;  /* 64 bit floating point value */
  typedef long double         LDOUBLE; /* 80 bit floating point value */

  typedef BYTE*               BYTEPTR;
  typedef WORD*               UINTPTR;
  typedef DWORD*              DWORDPTR;
  typedef void*               VOIDPTR;
#endif /* EMU_WINDOWS */

/********************************************************************
* This 16 bit unsigned value is used for routines which return
* standard E-mu error codes (see emuerrs.h)
********************************************************************/
typedef unsigned short        EMUSTAT;

/********************************************************************
* Compare <limits.h> values with SoundFont Enabler limits.  Fail compile
* if discrepency. If compile fails due to one of these errors, then
* the SoundFont enabler will not work with your system or the state of your
* development environment.
********************************************************************/
#if (SCHAR_MAX != CHAR_MAXVAL) || (UCHAR_MAX != BYTE_MAXVAL)
  #error <datatype.h>: char is out of range!
#endif

#if (SHRT_MAX != SHRT_MAXVAL)
  #error <datatype.h>: short is out of range!
#endif

#if (LONG_MAX != LONG_MAXVAL)
  #error <datatype.h>: long is out of range!
#endif

#if (ULONG_MAX != DWORD_MAXVAL)
  #error <datatype.h>: unsigned long is out of range!
#endif

#ifdef __BYTE_COHERENT

/********************************************************************
* Convenient union datatypes to use for byte swapping or extracting
* bytes and/or shorts within a short or long value without using
* bit shifting. 
********************************************************************/
typedef struct twoBytesTag
{
  BYTE by0;
  BYTE by1;
} twoBytes;


typedef struct fourBytesTag
{
  BYTE by0;
  BYTE by1;
  BYTE by2;
  BYTE by3;
} fourBytes;


typedef struct twoWordsTag
{
  WORD w0;
  WORD w1;
} twoWords;

#elif defined(__BYTE_INCOHERENT)

typedef struct twoBytesTag
{
  BYTE by1;
  BYTE by0;
} twoBytes;


typedef struct fourBytesTag
{
  BYTE by3;
  BYTE by2;
  BYTE by1;
  BYTE by0;
} fourBytes;


typedef struct twoWordsTag
{
  WORD w1;
  WORD w0;
} twoWords;

#endif  /*  BYTE COHERENCY  */


typedef union twoByteUnionTag
{
  twoBytes byVals;
  WORD     wVal;
} twoByteUnion;


typedef union fourByteUnionTag  /* make the three 32 bit definitions  */
{                               /* interchangeable                    */
  fourBytes byVals;
  twoWords  wVals;
  DWORD     dwVal;
} fourByteUnion;

#endif /* __DATATYPE_H */
