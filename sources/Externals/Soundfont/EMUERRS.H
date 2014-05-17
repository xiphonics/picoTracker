     /*********************************************************************
     
     emuerrs.h
     
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
*  @(#)emuerrs.h	1.1 12:06:28 3/15/95 12:06:35
*
* Filename: emuerrs.h
*
* Description: Header file for Enabler Error Codes                 
*
*******************************************************************************
*/

#ifndef __EMUERRS_H
#define __EMUERRS_H

/*************
** Includes
*************/

#include "DATATYPE.H"

/************
** Defines
************/

#define SUCCESS  0

/***********************
** Enumeration Tables
***********************/

namespace soundfont {

/* last value 0 0x0 */

enum RIFFERRORTAG
{
  RIFF_IDERROR = SUCCESS + 1,
  RIFF_ERROR,
  RIFF_FINDERROR,
  RIFF_OPENFILEERROR,
  RIFF_READFILEERROR,
  RIFF_WRITEFILEERROR
};

/* last value 6 0x6 */

enum SFERRORTAG
{
  SF_ERROR = RIFF_WRITEFILEERROR + 1,
  SF_BUFFERERROR,
  SF_MEMORYERROR,
  SF_INVALIDBANK,
  SF_BANKLINKERROR,
  SF_BANKLOADERROR,
  SF_WRONGWAVETABLE,
  SF_PRESETNOTFOUND
};

} 
/**************************
** External Declarations
***************************/

/*
Create your own custom error messages to be indexed by the above
enumeration table.

I.E.

 char sfEr1[80];
 strcpy(sfEr1, "This file could not be opened\n");
 emuErrors[RIFF_OPENFILEERROR] = sfEr1;
 ...
 stat = EmuFunction();
 if (stat != SUCCESS) {
   printf("%s\n", emuErrors[stat]);
   return(-1);
   }

*/

extern CHAR* emuErrors[];

#endif /* __EMUERRS_H */

