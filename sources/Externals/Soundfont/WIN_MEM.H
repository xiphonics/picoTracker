     /*********************************************************************
     
     win_mem.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
/////////////////////////////////////////////////////////////////////
//                           WIN_MEM.H
//
// This module is a dispatcher for various types of operating system
// memory management. It was built in order to handle
// allocation, initialization, and block copies of large segments 
// under operating systems which use segmented memory
// and have special routines for handling large amounts of contiguous
// memory.  
//
// For example, under Windows the routines herein must use the 'huge'
// keyword to avoid the problems associated with 'far' pointers.
// Even though far pointers (in Intel-land) have an extent of 32
// bits, the offset of the pointer will wrap once it exceeds
// 0xFFFF.  In other words, the segment address will _not_
// increment as you might expect.  Therefore we must normalize
// the pointers with the 'huge' keyword.  A huge pointer, like a
// far pointer, has an extent of 32 bits, but because it is
// normalized, the segment will have a value from 0 to 15 since
// a segment can start every 16 bytes.  Please note that there
// is only one possible huge address, or more exactly,
// only one Segment:Offset pair.
//
// The methods are selected at COMPILE TIME using one of the following
// compile flags:
//
//    EMU_WINDOWS:    Huge pointers allocated from MS-Windows GLOBAL heap 
//    DOS_MEMORY:     Huge pointers allocated using farmalloc under MS-DOS
//    CPP_MEMORY:     Normal pointers for systems without segmented
//                    archetecture issues in memory management which are
//                    left up to developers to solve (example: UNIX)
//
/////////////////////////////////////////////////////////////////////
#ifndef __WIN_MEM_H
#define __WIN_MEM_H

/////////////////////////////
//       Includes          //
/////////////////////////////

#include <stddef.h>


/////////////////////////////
//        Defines          //
/////////////////////////////

#define DEBUG_MEMORY 0


//////////////////////////////////////////////////////////////////
// Please notate the desired memory manager by defining it 
// for the desired memory routines for your environment.
// You _must_ place one of the following defines in your make
// file.
//   #define CPP_MEMORY        
//   #define EMU_WINDOWS
//   #define DOS_MEMORY    
//   #define MAC_MEMORY           // Not implemented at this point
//////////////////////////////////////////////////////////////////

/////////////////////////////
//       Declarations      //
/////////////////////////////

////////////////////////////////////////////////////////////////
// These overloaded functions are only applicable under Windows
////////////////////////////////////////////////////////////////
#if defined(EMU_WINDOWS)
#include <windows.h>           // The all-important windows include file
#include "DATATYPE.H"

VOIDPTR Alloc(DWORD dwSize);
VOIDPTR Memset(VOIDPTR pMem, WORD wValue, DWORD dwSize);
void    Dealloc(VOIDPTR pMem);
VOIDPTR HFMemCopy(VOIDPTR pDest, VOIDPTR pSrc, DWORD dwNBytes);

////////////////////////////////////////////////////////////////
// The following is normal memory management using new/delete
////////////////////////////////////////////////////////////////
#elif defined(CPP_MEMORY)

#include "DATATYPE.H"

VOIDPTR Alloc(DWORD dwSize);
VOIDPTR Memset(VOIDPTR pMem, WORD wValue, DWORD dwSize);
void    Dealloc(VOIDPTR pMem);
VOIDPTR HFMemCopy(VOIDPTR pDest, VOIDPTR pSrc, DWORD dwNBytes);

////////////////////////////////////////////////////////////////
// The following is specific for MS-DOS memory management
////////////////////////////////////////////////////////////////
#elif defined(DOS_MEMORY)

#include <alloc.h>
#include "DATATYPE.H"

VOIDPTR Alloc(DWORD dwSize);
VOIDPTR Memset(VOIDPTR pMem, WORD wValue, DWORD dwSize);
void    Dealloc(VOIDPTR pMem);
VOIDPTR HFMemCopy(VOIDPTR pDest, VOIDPTR pSrc, DWORD dwNBytes);

#endif // DOS_MEMORY


#endif // __WIN_MEM_H
//////////////////////// End of WIN_MEM.H ///////////////////////////
