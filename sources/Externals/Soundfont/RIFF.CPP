/*********************************************************************

riff.cpp

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



/*****************************************************************************
*  @(#)riff.cpp 1.1 12:06:44 3/15/95 12:06:47
*
* Filename: RIFF.CPP
*
* Description:  Basic methods to read RIFF files
*
* 
* Visible Routines: 
*  
* 
* [Local Routines:] 
*
* NOTES: All file/data stream access requests are made through 
*        RIFFOpen, RIFFClose, RIFFRead, RIFFSeek, RIFFTell.
*
*        Inside those routines are the methods relevant to your file
*        system I/O commands.   
*
*********************************************************************
*/

#ifndef __RIFF_CPP
#define __RIFF_CPP

#include <errno.h>
#include <ctype.h>
/////////////////////////////
//        Includes         //
/////////////////////////////

#include "RIFF.H"
#include "System/Console/Trace.h"
#include <stdio.h>

using namespace::soundfont ;

/////////////////////////////
//          Defines        //
/////////////////////////////

static CHAR RIFF[5] = { 'R', 'I', 'F', 'F', '\0' };
static CHAR SFBK[5] = { 's', 'f', 'b', 'k' ,'\0'};
//static CHAR SFbk[4] = { 'S', 'F', 'b', 'k' ,'\0'};
static CHAR LIST[5] = { 'L', 'I', 'S', 'T' ,'\0'};
static CHAR INFO[5] = { 'I', 'N', 'F', 'O' ,'\0'};

static CHAR IFIL[5] = { 'i', 'f', 'i', 'l' ,'\0'};
static CHAR ISNG[5] = { 'i', 's', 'n', 'g','\0' };
static CHAR IROM[5] = { 'i', 'r', 'o', 'm','\0' };
static CHAR IVER[5] = { 'i', 'v', 'e', 'r','\0' };
static CHAR INAM[5] = { 'I', 'N', 'A', 'M','\0' };
static CHAR IPRD[5] = { 'I', 'P', 'R', 'D','\0' };
static CHAR ICOP[5] = { 'I', 'C', 'O', 'P','\0' };
static CHAR ICRD[5] = { 'I', 'C', 'R', 'D','\0' };
static CHAR IENG[5] = { 'I', 'E', 'N', 'G','\0' };
static CHAR ICMT[5] = { 'I', 'C', 'M', 'T','\0' };  // new comment subChunk

static CHAR SDTA[5] = { 's', 'd', 't', 'a','\0' };
static CHAR SNAM[5] = { 's', 'n', 'a', 'm','\0' };
static CHAR SMPL[5] = { 's', 'm', 'p', 'l','\0' };

static CHAR PDTA[5] = { 'p', 'd', 't', 'a','\0' };
static CHAR PHDR[5] = { 'p', 'h', 'd', 'r','\0' };
static CHAR PBAG[5] = { 'p', 'b', 'a', 'g','\0' };
static CHAR PMOD[5] = { 'p', 'm', 'o', 'd','\0' };
static CHAR PGEN[5] = { 'p', 'g', 'e', 'n','\0' };
static CHAR INST[5] = { 'i', 'n', 's', 't','\0' };
static CHAR IBAG[5] = { 'i', 'b', 'a', 'g','\0' };
static CHAR IMOD[5] = { 'i', 'm', 'o', 'd','\0' };
static CHAR IGEN[5] = { 'i', 'g', 'e', 'n','\0' };
static CHAR SHDR[5] = { 's', 'h', 'd', 'r','\0' };

//// Other IDs ////
static CHAR WAVEID[5] = { 'W', 'A', 'V', 'E','\0' };
static CHAR FMTID[5]  = { 'f', 'm', 't', ' ','\0' };
static CHAR DATAID[5] = { 'd', 'a', 't', 'a','\0' };

//// Macintosh IDs ////
static CHAR AIFF[5]   = { 'A', 'I', 'F', 'F','\0' };



/////////////////////////////
//        Methods          //
/////////////////////////////

//////////////////////////
// The Constructor
//////////////////////////
RIFFClass::RIFFClass(void)
{
  ConstructRIFF();
}


//////////////////////////
// The Destructor
//////////////////////////

RIFFClass::~RIFFClass(void)
{ 
  DestructRIFF();
}

DWORD MAKE_ID(char* riffID)
{
  DWORD retval;
  memcpy(&retval, riffID, 4);
  return retval;
}

////////////////////////////
// The Callable Constructor
////////////////////////////
void RIFFClass::ConstructRIFF(void)
{
  pFile             = NULL;
  dwLastID          = 0;
  dwLastSize        = 0;
  dwRIFFSize        = 0;
  lLastFilePtr      = 0;
  dwLastFormID      = 0;

  initRiffTokensArray(); 
}


////////////////////////////
// The Callable Destructor
////////////////////////////
void RIFFClass::DestructRIFF(void)
{
  /// Close file here ////
  CloseRIFF();
}


//*********************************************************
// The RIFF Tokens Array is the collection of RIFF ID tokens that this
// module recognizes. Here the array is initialized.
//*********************************************************
SHORT RIFFClass::initRiffTokensArray()
{
  // Riff Header Chunks Tokens...

 memset(RiffTokens,0,RiffCkCount);
 

 RiffTokens[Riff] = RIFF;
 RiffTokens[Sfbk] = SFBK; 
 RiffTokens[List] = LIST; 
 RiffTokens[Info] = INFO; 

       // Info Chunks Tokens...

 RiffTokens[Ifil] = IFIL;
 RiffTokens[Isng] = ISNG;
 RiffTokens[Irom] = IROM;
 RiffTokens[Iver] = IVER;
 RiffTokens[Inam] = INAM;
 RiffTokens[Iprd] = IPRD;
 RiffTokens[Icop] = ICOP;
 RiffTokens[Icrd] = ICRD;
 RiffTokens[Ieng] = IENG;
 RiffTokens[Icmt] = ICMT;

       // Sample Chunk Tokens...

 RiffTokens[Sdta] = SDTA;
 RiffTokens[Snam] = SNAM;
 RiffTokens[Smpl] = SMPL;

      // Preset Chunk Tokens...

 RiffTokens[Pdta] = PDTA;
 RiffTokens[Phdr] = PHDR;
 RiffTokens[Pbag] = PBAG;
 RiffTokens[Pmod] = PMOD;
 RiffTokens[Pgen] = PGEN;
 RiffTokens[Inst] = INST;
 RiffTokens[Ibag] = IBAG;
 RiffTokens[Imod] = IMOD; 
 RiffTokens[Igen] = IGEN;
 RiffTokens[Shdr] = SHDR;

 return SUCCESS;

}// end initTokensArray

//*********************************************************
//  The methods which open and read the RIFF files       
//
//  Note:  When we ReadCkHdr, the file ptr is always     
//  left at the beginning of the chunk data.  When we    
//  DescendRIFF, the file ptr is left at the beginning   
//  of the data as well.  When we use AscendRIFF,        
//  the same file ptr positioning applies.  AscendRIFF   
//  will take us to the next beginning of the next chunk 
//  since we are emerging from the present chunk, but    
//  DescendRIFF will not keep descending unless          
//  subsequent chunks have a LIST ID, since simple       
//  non-RIFF and non-LIST chunks may not contain         
//  subchunks.  Id est, there is  nothing more to        
//  descend into.                                        
//*********************************************************

//*********************************************************
// This is a call to open a file and verify that it is 
// a RIFF file.
//*********************************************************
WORD RIFFClass::OpenRIFF(CHAR* pName)
{
  byWhereIsRIFFData = RIFF_ONDISK;

  if ((RIFFOpen((LONG)pName) != SUCCESS ) || (pFile == NULL))
  { 
     SetError(errno); // Set inside RIFFOpen
     return (uiErrorNdx = RIFF_OPENFILEERROR);
  }

  return (InitRIFF()); 
  
}
 
//*********************************************************
// Call to open and verify a file is a RIFF File.
// This method uses Macintosh File I/O
//*********************************************************
#ifdef USE_MACINTOSH
WORD RIFFClass::OpenRIFF(FSSpec* pSpecifier)
{
  byWhereIsRIFFData = RIFF_ONMACDISK;
 
  if (RIFFOpen((LONG)pSpecifier) != SUCCESS)
    return (RIFF_OPENFILEERROR);
   
  return(InitRIFF());
}
#endif // USE_MACINTOSH


//*********************************************************
// Call to verify that the file opened is a RIFF file and
// to leave the current file position at the Format ID
// position.
//*********************************************************
WORD  RIFFClass::InitRIFF(void)
{

  if (ReadCkHdr() != SUCCESS)
	 return (uiErrorNdx = RIFF_OPENFILEERROR);
  else if (dwLastID != MAKE_ID(RIFF))
	 return (uiErrorNdx = RIFF_IDERROR);

  dwRIFFSize = dwLastSize;
  return (uiErrorNdx = SUCCESS);

}

//******************************************************
// Call to close the file.
//******************************************************
WORD  RIFFClass::CloseRIFF(void)
{
  uiErrorNdx = SUCCESS;
  
  if (0 != RIFFClose())
	 uiErrorNdx = RIFF_ERROR;
  return (uiErrorNdx);
}


//*************************************************
// Reads the chunk header and retains their values
//*************************************************
WORD  RIFFClass::ReadCkHdr(void)
{
  ckHeader ckHdr;

  if (RIFFRead(&ckHdr, sizeof(ckHdr), 1) != 1)
	 return (RIFF_READFILEERROR);

  char RIFFDebug[5];
  RIFFDebug[4] = 0;

  char *chIdPtr=(char *)&(ckHdr.dwCkID) ;
  for (int i=0;i<4;i++) 
  {
	  RIFFDebug[i] = *chIdPtr = toupper(*chIdPtr) ;
	  chIdPtr++ ;
  } ;
  dwLastID   = ckHdr.dwCkID;
  dwLastSize = ckHdr.dwCkSize;
  SwapDWORD(&dwLastSize);

  if ((dwLastID == MAKE_ID(RIFF)) || (dwLastID == MAKE_ID(LIST))) {
    
    RIFFRead(&dwLastFormID, sizeof(dwLastFormID), 1);
  	chIdPtr=(char *)&dwLastFormID ;
	  for (int i=0;i<4;i++) {
		  *chIdPtr=toupper(*chIdPtr) ;
		  chIdPtr++ ;
	  } ;
  } else {
	 dwLastFormID = 0;
  }
  lLastFilePtr = RIFFTell();

  return (SUCCESS);
}


//***********************************************************
// Descend into a chunk, i.e., a set of one or more subchunks
// contained in a parent chunk
//***********************************************************
WORD  RIFFClass::Descend(void)
{
  //// We are at a RIFF or LIST ID ////
  if ((dwLastID == MAKE_ID(RIFF)) || (dwLastID == MAKE_ID(LIST)))
  {
    return (ReadCkHdr());
  }
  //// There are no more RIFF or LIST chunks within this chunk ////
  else
  {
    return (RIFF_ERROR);
  }
}


//************************************************************
// Ascend to the next chunk, i.e., skip to the very next chunk
//************************************************************
WORD  RIFFClass::Ascend(void)
{
  if (((DWORD) lLastFilePtr + dwLastSize) <= dwRIFFSize)
  {
	 //// Our last chunk had a RIFF or LIST ID ////
	 if (dwLastFormID)
		dwLastSize -= sizeof(DWORD);

	 if ((RIFFSeek(lLastFilePtr + (LONG) dwLastSize, SEEK_SET)) == 0)
		return (ReadCkHdr());
	 else
		return (RIFF_READFILEERROR);
  }
  else
	 return (RIFF_ERROR);
}


//**************************************
// Find a subchunk with the pID identity
//**************************************
WORD  RIFFClass::FindCk(CHAR* pID)
{
  char tmp[4] ;
  for (int i=0;i<4;i++) 
  {
	  tmp[i]=toupper(*pID++) ;
  }
  DWORD dwID     = MAKE_ID(tmp);

  Reset();
  ReadCkHdr();

  while (TRUE)
  {
	  if ((dwID == dwLastID) ||        // We found our chunk
	      (dwID == dwLastFormID))
    {
  		return (SUCCESS);
    }
	  else if (Descend() != SUCCESS)
	  {
		  if (Ascend() != SUCCESS)
      {       // Jump to the next chunk
      	break;
      }
 	  }
  }
  return (RIFF_FINDERROR);
}


//***************************************************
// Move the file pointer back to the beginning
//***************************************************
WORD  RIFFClass::Reset(void)
{
  errno = 0; 
  if ((RIFFSeek(0L, SEEK_SET)) != 0)
  {
    return (RIFF_READFILEERROR);
  }
  else
  {
    return (ReadCkHdr());
  }
}



DWORD  RIFFClass::GetRIFFSize(void) { return (dwRIFFSize);   }

DWORD  RIFFClass::GetCkID(void)     { return (dwLastID);     }

DWORD  RIFFClass::GetCkSize(void)   { return (dwLastSize);   }

DWORD  RIFFClass::GetCkFormID(void) { return (dwLastFormID); }



//***************************************************
//  Functions to navigate the storage medium itself
//  Modes exist for ANSI File I/O and Macintosh File I/O
//  If ANSI File I/O, lPointer is a pointer to a 
//  character string. If Macintosh File I/O, lPointer
//  is a pointer to the File System Specifier (FSSpec)
//  data structure.
//***************************************************
SHORT RIFFClass::RIFFOpen(LONG lPointer)
{
  //***************************************************
  // Keep in mind that whenever a system call is made and
  // _no_ error takes place, errno is _not_ set to 0.
  // Obviously, this means that any previous system call
  // will set errno upon an error, one which is innocuous,
  // but will cause fopen to _appear_ as though it failed.
  // For example, open a temporary file which might not
  // exist:
  //
  //   fopen("tempfile", "wb");
  //
  // If it doesn't exist, errno become 2 (in DOS, at
  // least).  Now call the fopen below for a preexisting
  // RIFF file.  Guess what!  If you didn't reset errno
  // to 0, your return value will be 2, an apparent error.
  //***************************************************
  errno = 0;

  switch (byWhereIsRIFFData)
  {
    case RIFF_ONDISK:
      pFile = fopen((CHAR *)lPointer, "rb");
      return (errno);

  #ifdef USE_MACINTOSH
    case RIFF_ONMACDISK:
      return (FSpOpenDF((FSSpec *)lPointer, fsRdPerm, &fRefNum));
  #endif

    default:
      return (1);
  }
}

SHORT RIFFClass::RIFFClose()
{
  switch (byWhereIsRIFFData) 
  {
    case RIFF_ONDISK:
    {
      SHORT sStat = SUCCESS;

      if (pFile)
      {
	sStat = fclose(pFile);
	pFile = 0;
      }
      return (sStat);
    }
  #ifdef USE_MACINTOSH
    case RIFF_ONMACDISK:
      return (FSClose(fRefNum));
  #endif

    default:
      return (1);
  }
}

SHORT RIFFClass::RIFFRead(VOIDPTR vStream, WORD wSize, WORD wNum)
{
  switch (byWhereIsRIFFData) {

  case RIFF_ONDISK:
    SHORT err;
    err = fread(vStream, (size_t)wSize, (size_t)wNum, pFile);
    if (ferror(pFile) != 0)
      return 0;
    return err;  
    // break;

  #ifdef USE_MACINTOSH
  case RIFF_ONMACDISK:
    LONG size = (LONG)wSize * (LONG)wNum;
    LONG saveSize = size;
    FSRead(fRefNum, &size, vStream);
    if (saveSize == size)
      return wNum; 
    return 0;
    //break;
  #endif

  default:
    return(1);
    //break;
  }
}

SHORT
RIFFClass::RIFFSeek(LONG lOffset, SHORT shWhence)
{
  switch (byWhereIsRIFFData) {

  case RIFF_ONDISK:
    return (fseek(pFile, lOffset, shWhence));
    //break;  

  #ifdef USE_MACINTOSH
  case RIFF_ONMACDISK:
    if      (shWhence == SEEK_SET) shWhence = fsFromStart; // 0 -> 1
    else if (shWhence == SEEK_CUR) shWhence = fsFromMark;  // 1 -> 3
    else if (shWhence == SEEK_END) shWhence = fsFromLEOF;  // 2 -> 2
    return (SetFPos(fRefNum, shWhence, lOffset));
    //break;
  #endif

  default:
    return(-1);
}
}

DWORD  RIFFClass::RIFFTell()
{
  switch (byWhereIsRIFFData) {

  case RIFF_ONDISK:
    return (ftell(pFile));
    //break;  

  #ifdef USE_MACINTOSH
  case RIFF_ONMACDISK:
    LONG lPos = 0;
    GetFPos(fRefNum, &lPos);
    return lPos;
    //break;
  #endif

  default:
    return(1);
  }
}

DWORD
RIFFClass::RIFFTellAbs(void)
{
  return(RIFFTell());
}

FILE*  RIFFClass::GetFilePtr(void) const { return (pFile);   }

//**********************************************************************
// Methods for BYTE SWAPPING. Since a RIFF file is Little Endian
// or Byte Coherent by nature, Big Endian or Byte Incoherent machines
// need to swap bytes around BEFORE looking at data.
//**********************************************************************
#ifdef __BYTE_INCOHERENT
void  RIFFClass::SwapBytes(WORD* pDWORD)
{

  twoByteUnion unUINT, unSwap;

  unSwap.wVal      = *pDWORD;
  unUINT.byVals.by0 = unSwap.byVals.by1;
  unUINT.byVals.by1 = unSwap.byVals.by0;
  *pDWORD            = unUINT.wVal;
 
}


void  RIFFClass::SwapDWORD(DWORD* pDWORD)
{

  fourByteUnion unDWORD, unSwap;

  unSwap.dwVal       = *pDWORD;
  unDWORD.byVals.by0 = unSwap.byVals.by3;
  unDWORD.byVals.by1 = unSwap.byVals.by2;
  unDWORD.byVals.by2 = unSwap.byVals.by1;
  unDWORD.byVals.by3 = unSwap.byVals.by0;
  *pDWORD            = unDWORD.dwVal;

}

#elif defined (__BYTE_COHERENT)
void  RIFFClass::SwapBytes(WORD*)
{
  return;
}

void  RIFFClass::SwapDWORD(DWORD*)
{
  return;
}

#endif



#endif // __RIFF_CPP

