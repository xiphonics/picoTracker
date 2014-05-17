     /*********************************************************************
     
     win_mem.cpp
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
//*****************************************************************************
//
//                          Copyright (c) 1994
//               E-mu Systems Proprietary All rights Reserved.
//                             
//*****************************************************************************

#include <string.h>
#include "DATATYPE.H"
#include "WIN_MEM.H"

//*****************************************************************************
// @(#)win_mem.cpp	1.2 13:56:17 3/22/95 13:56:17
//
// Description: 
//
// Routines: 
// 
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// Robert S. Crawford  12/6/94        Initial Creation             
//*****************************************************************************


/////////////////////////////
//      Definitions        //
/////////////////////////////


///////////////////////////////////////////////////////
// Take note that these functions are only applicable
// when complied under Windows.
///////////////////////////////////////////////////////

#if defined(WINDOWS_MEMORY)

//////////////////////////
// Allocate your memory
//////////////////////////
VOIDPTR Alloc(DWORD dwSize)
{
  DWORD      dwAllocated;
  HANDLE     hMem;
  void huge* pMem;

  ///////////////////////////////////////////////////
  // Obtain a handle to the allocated memory.  Also
  // be aware that the we are allocating shorts, not
  // bytes.  Therefore, the number of bytes needed
  // will be twice the number of samples indicated.
  ///////////////////////////////////////////////////
  hMem = GlobalAlloc(GHND, (DWORD)(dwSize));

  //// Did we manage to get any memory allocated? ////
  if (hMem != NULL)
  {
    dwAllocated = GlobalSize(hMem);

    //// Was enough memory allocated? ////
    if (dwSize <= dwAllocated)
    {
      //// Lock it in place ////
      pMem = (VOIDPTR)GlobalLock(hMem);

      if (pMem != NULL)
      {
	return (pMem);
      }
    }

    //// Free up the allocated memory since you didn't pass your tests ////
    GlobalFree(hMem);
  }

  return (NULL);
}


////////////////////////////////////////
// Initialize your memory to some value
////////////////////////////////////////
VOIDPTR Memset(VOIDPTR pMem, WORD wValue, DWORD dwSize)
{
  if (pMem != NULL)
  {
    BYTEPTR pByte = (BYTEPTR) pMem;

    if (dwSize != 0)
    {
      //// Decrement for zero-based indexing ////
      dwSize--;

      while (dwSize > 0)
      {
	pByte[dwSize] = wValue;
	dwSize--;
      }

      //// Pick up the last byte ////
      pByte[0] = wValue;
    }
  }
  return (pMem);
}


//////////////////////////
// Deallocate your memory
//////////////////////////
void Dealloc(VOIDPTR pMem)
{
  HANDLE hMem;

  ///////////////////////////////
  // Memory debugging code
  ///////////////////////////////

  if (pMem != NULL)
  {
    hMem = GlobalHandle(SELECTOROF(pMem));

    if (hMem != NULL)
    {
      GlobalUnlock(hMem);
      GlobalFree(hMem);

      pMem = NULL;
    }
  }
}


///////////////////////////////////////////////
// Copy your memory to another memory location
///////////////////////////////////////////////
VOIDPTR HFMemCopy(VOIDPTR pDest, VOIDPTR pSrc, DWORD dwNBytes)
{
  //// A Windows 3.1 specific call... ////
  hmemcpy(pDest, pSrc, (LONG) dwNBytes);

  /////////////////////////////////////////////////
  // Here's how one might code the above function:
  /////////////////////////////////////////////////
  /*
  DWORD   dwBytesLeft;
  BYTEPTR pTempSrc    = (BYTEPTR) pSrc;
  BYTEPTR pTempDest   = (BYTEPTR) pDest;


  if (dwNBytes != 0)
  {
    //// Decrement for zero-based indexing ////
    dwNBytes--;

    while (dwNBytes > 0)
    {
      pTempDest[dwNBytes] = pTempSrc[dwNBytes];
      dwNBytes--;
    }

    //// Pick up the last byte ////
    pTempDest[0] = pTempSrc[0];
  }
  */

  return ((VOIDPTR) pDest);
}


////////////////////////////////////////////////////////////////
// The following is normal memory management using new/delete
////////////////////////////////////////////////////////////////
#elif defined(CPP_MEMORY)

VOIDPTR Alloc(DWORD dwSize)
{
  return ((VOIDPTR) new BYTE[(size_t) dwSize]);
}


VOIDPTR Memset(VOIDPTR pMem, WORD wValue, DWORD dwSize)
{
  return (memset(pMem, wValue, dwSize));
}


void Dealloc(VOIDPTR pMem)
{
  delete [] pMem;
  pMem = 0;
}


VOIDPTR HFMemCopy(VOIDPTR pDest, VOIDPTR pSrc, DWORD dwNBytes)
{
  return (memcpy(pDest, pSrc, (size_t)dwNBytes));
}

////////////////////////////////////////////////////////////////
// The following is DOS memory management using far... functions
////////////////////////////////////////////////////////////////
#elif defined(DOS_MEMORY)

VOIDPTR Alloc(DWORD dwSize)
{
  return ((VOIDPTR)farmalloc(dwSize));
}


VOIDPTR Memset(VOIDPTR pMem, WORD wValue, DWORD dwSize)
{
  VOIDPTR theReturn = pMem;
  BYTEPTR pByte = (BYTEPTR) pMem;
  WORD count = 0;
  size_t theMemSetSize;
  while (dwSize != 0)
  {
   if (dwSize > 0xFFFF)
     theMemSetSize = 0xFFFF;
   else
     theMemSetSize = (size_t)dwSize;
   if (_fmemset(&pByte[count*0x10000], wValue, theMemSetSize) == NULL)
     return NULL;
   dwSize -= theMemSetSize;
   count++;
  }
  return theReturn;
}


void Dealloc(VOIDPTR pMem)
{
  farfree(pMem);
  pMem = 0;
}


VOIDPTR HFMemCopy(VOIDPTR pDest, VOIDPTR pSrc, DWORD dwNBytes)
{
  BYTEPTR pByteDest;
  BYTEPTR pByteSrc;

  pByteDest = (BYTEPTR)pDest;
  pByteSrc =  (BYTEPTR)pSrc;

  VOIDPTR theReturn = (VOIDPTR)(pByteDest[dwNBytes]);
  DWORD count = 0;
  size_t theMemCpySize;
  while (dwNBytes != 0)
  {
   if (dwNBytes > 0xFFFF)
     theMemCpySize = 0xFFFF;
   else
     theMemCpySize = (size_t)dwNBytes;
   if (_fmemcpy(&pByteDest[count*0x10000], &pByteSrc[count*0x10000], theMemCpySize) == NULL)
     return NULL;
   dwNBytes -= theMemCpySize;
   count++;
  }
  return theReturn;

}

#endif

//////////////////////// End of WIN_MEM.CPP /////////////////////////
