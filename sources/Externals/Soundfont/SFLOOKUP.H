     /*********************************************************************
     
     sflookup.h
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
#ifndef __SFLOOKUP_H
#define __SFLOOKUP_H

#include "DATATYPE.H" 

#define MEMBEROFFSET(typ, mbr) \
	  ((WORD) ((CHAR*) &((typ*) 0)->mbr - (CHAR*) 0))

#ifdef _UNIX_STUB_
typedef DWORD SFLOOKUP;
#else
typedef WORD SFLOOKUP;
#endif

void InitLookup(void);
void ClearLookup(void);
const SFLOOKUP *GetLookup(void);

#endif
