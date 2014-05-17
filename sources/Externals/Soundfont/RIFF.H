     /*********************************************************************
     
     riff.h
     
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

#ifndef __RIFF_HPP
#define __RIFF_HPP


#include "DATATYPE.H"
#include "EMUERRS.H"
#include "WIN_MEM.H"
#include "OMEGA.H"

#ifdef USE_MACINTOSH
//*********************************************************************
// This is the file which contains the prototypes for the Macintosh 
// file I/O streams. It contains definitions of the FSSpec pointer as
// well as the prototypes for the I/O commands and related enumerations.
// Reference: Files.h Version 2.0a3 ETO #16,
//            MPW prerelease. Friday, November 11, 1994.
//*********************************************************************
#include "FILES.H"
#endif

#include <stdio.h>
#include <string.h>

//*****************************************************************************
// @(#)riff.h	1.1 12:06:31 3/15/95 12:06:36
//                             
// Filename: RIFF.H
//
//********************************************************************
 


/////////////////////////////////////////////////
//      SoundFont and RIFF typedefs            //
/////////////////////////////////////////////////

typedef struct ckHeaderTag
{
  DWORD dwCkID;
  DWORD dwCkSize;
} ckHeader;


typedef struct ckFormTag
{
  ckHeader ckHdr;
  DWORD    dwFormID;
} ckForm;


typedef struct ckListTag
{
  ckHeader  ckHdr;
  DWORD     dwListID;
} ckList;


/////////////////////////////
//        Defines          //
/////////////////////////////

#define FORMSIZE             ((LONG) sizeof(ckForm))
#define HDRSIZE              ((LONG) sizeof(ckHeader))
#define TYPESIZE             ((LONG) sizeof(DWORD))

#define MAKE_IDSTR(str, id)  (strncpy(str, & (CHAR) (id), 4))

#define RIFF_ONDISK 0
#define RIFF_ONMACDISK 3

/////////////////////////////
//       Statics           //
/////////////////////////////

DWORD MAKE_ID(char *);

/////////////////////////////
//       Enumerations      //
/////////////////////////////

typedef enum {

// Riff header Tokens...

     Riff,
     Sfbk, 
     List,
     Info,
     
// Info Chunk Tokens...

     Ifil,
     Isng,
     Irom,
     Iver,
     Inam,
     Iprd,
     Icop,
     Icrd,
     Ieng,
     Icmt,

// Sample Chunk Tokens

     Sdta,
     Snam, 
     Smpl,

// Preset Chunk Tokens

     Pdta,
     Phdr,
     Pbag,
     Pmod, 
     Pgen, 
     Inst,
     Ibag,
     Imod,
     Igen,
     Shdr

}RiffCkTokenType;

#define RiffCkCount 28


/////////////////////////////
//         Classes         //
/////////////////////////////

class RIFFClass
{
  public:
    RIFFClass(void);
    ~RIFFClass(void);

    void ConstructRIFF(void);
    void DestructRIFF(void);

    #ifdef USE_MACINTOSH
    WORD        OpenRIFF(FSSpec * pFileSpecifier);
    #endif
    
    WORD        OpenRIFF(CHAR* pName); 
    WORD  CloseRIFF(void);
    WORD  ReadCkHdr(void);
    WORD  Descend(void);
    WORD  Ascend(void);
    WORD  FindCk(CHAR* pID);
    WORD  Reset(void);

    DWORD  GetRIFFSize(void);
    DWORD  GetCkID(void);
    DWORD  GetCkSize(void);
    DWORD  GetCkFormID(void);
    WORD   GetErrorNdx() { return GetError(); }

    void  SwapBytes(WORD* pWORD);
    void  SwapDWORD(DWORD* pDWORD);


    BYTE   byWhereIsRIFFData;
    FILE*  GetFilePtr(void) const;
    WORD   uiErrorNdx;
    
    SHORT    RIFFWrite(const VOIDPTR, WORD wSize, WORD wNUM);

    SHORT    RIFFRead(VOIDPTR buf, WORD wSize, WORD wNum);

    SHORT    RIFFSeek(LONG lOffset, SHORT shWhence);

    DWORD    RIFFTell(void); 
    DWORD    RIFFTellAbs(void);
    SHORT    RIFFOpen(LONG);
    SHORT    RIFFClose(void);
	

    SHORT    initRiffTokensArray();

    CHAR*  GetRIFFToken(WORD wCurToken)
	      { return (RiffTokens[wCurToken]); }


    void   SetError(SHORT shError) { omega.SetError(shError);    }
    SHORT  GetError(void)          { return omega.GetError();    } 
    BOOL   IsOK(void)              { return omega.IsOK();        }
    BOOL   IsBad(void)             { return omega.IsBad();       }
    void   ReportError(void) ;


 
  private:
    WORD    InitRIFF(void);
    FILE*   pFile;

    #ifdef USE_MACINTOSH
    SHORT   fRefNum;
    #endif
    
    LONG  lLastFilePtr;    // The chunk in the file where we visited last
    DWORD dwLastID;        // The ID of the last RIFF/LIST chunk visited
    DWORD dwLastSize;      // The size of our last visited chunk
    DWORD dwLastFormID;    // The last form/list-type we visited
    DWORD dwRIFFSize;      // The size of the file - the header ID and Size

    CHAR *RiffTokens[RiffCkCount];

    OmegaClass omega; 


};



#endif // __RIFF_HPP
////////////////////////// End of RIFF.HPP //////////////////////////
