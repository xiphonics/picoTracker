     /*********************************************************************
     
     hydra.h
     
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
#ifndef __HYDRA_H_
#define __HYDRA_H_

#include <stddef.h>
#include "DATATYPE.H"
#include "SFDATA.H"
#include "SFENUM.H"
#include "EMUERRS.H"
//*****************************************************************************
// @(#)hydra.h	1.2 09:40:05 3/21/95 09:40:06
// Description: 
//
//     This file defines the datatype HydraClass. 
// HydraClass is the base level in memory datatype used to describe
// a single SoundFont bank. It also contains the Global default parameters for
// a SoundFontObject. 
// 
//*****************************************************************************

#define SF_DATASTRUCTS  9       // The # of SF data structs
#define SAMPLENAMESIZE  20      // The size of the strings in the Hydra

#define PRESETNAMESIZE  20      
#define INSTNAMESIZE    20

#define SAMPLEBUFFER    46

enum SoundFontDS {
  prstHdr = 0,
  prstBagNdx,
  prstGenList,
  prstModList,
  inst,
  instBagNdx,
  instGenList,
  instModList,
  sampHdr

};


/////////////////////////////
//SoundFont Data Structures//
/////////////////////////////

typedef struct sfVersionTag
{
  WORD wMajor;               // Reads the major ver (if present) -was uiHi
  WORD wMinor;               // Reads the minor ver              -was uiLo
} sfVersion;


typedef struct sfINFOTag
{
  sfVersion sfSFVer;
  CHAR*     pchSoundEngine;
  CHAR*     pchROMName;
  sfVersion sfROMVer;
  CHAR*     pchBankName;
  CHAR*     pchProduct;
  CHAR*     pchCopyright;
  CHAR*     pchDate;
  CHAR*     pchEngineer;
} sfINFO;

// The stRanges data structure is applicable in the Generator List only.
// They are used as values for key ranges and velocity ranges.
// Typically, a Generator value is a 16 bit signed entity, whose bytes must be
// swapped on Big Endian (Byte Incoherent) machines. In other words,
// the byte ordering for _typical_ generators is NOT the same 
// between Big Endian and Little Endian systems.

// However the byte ordering for _ranges_ IS the same regardless of the
// Endian nature of the system. 

// So instead of having the sfReader detect the relatively few key ranges 
// and NOT swapping those, it swaps bytes for ALL generator values 
// and these data structures are re-designed to compensate for that.

#ifdef __BYTE_COHERENT
struct stRanges            // This is for both key and velocity ranges
{
  BYTE byLo;    
  BYTE byHi;   
};

#elif defined (__BYTE_INCOHERENT)
struct stRanges
{
  BYTE byHi;  
  BYTE byLo;
};

#endif


typedef union unGenAmtTag
{
  struct stRanges stRange;    // Sometimes the generator is an stRange
  SHORT             shAmount; //   and sometimes it is a short
} unGenAmt;


//// Standard list used in Phdr (Preset lists) ////
typedef struct sfPresetHdrTag
{
  CHAR   achPresetName[PRESETNAMESIZE];      // Name of preset  
  WORD   wPresetNum;                   // The number of the preset
  WORD   wPresetBank;                 // The preset bank number
  WORD   wBagNdx;                    // The index to sfPresetBag
  DWORD  dwLibrary;
  DWORD  dwGenre;
  DWORD  dwMorphology;

} sfPresetHdr;



//// Standard list used in Pbag, Ibag (Layer, split lists) ////
typedef struct sfBagNdxTag

{
  WORD wGenNdx;          // Index to generator list
  WORD wModNdx;          // Index to modulator list

} sfBagNdx;


//// Standard list used in Pgen, Igen (Generator lists) ////
typedef struct sfGenListTag
{
  WORD         sfGenOper; // Single generator 
  unGenAmt     unAmt;     // Amount applied to that generator
} sfGenList;


//// Standard list used in Pmod, Imod (Modulator lists) ////
typedef struct sfModListTag
{
  WORD    wModSrcOper;     // Source modulation operator
  WORD    wModDestOper;    // Destination modulation operator
  SHORT   shAmount;        // Source modulates destination by this amount
  WORD    wModAmtSrcOper;  // If used, source modulates destination by 
                           // THIS source's current value instead.
  WORD    wModTransOper;   // Send source through defined transform first
} sfModList;


//// Standard list used in Inst (Instrument lists) ////
typedef struct sfInstTag
{
  CHAR  achInstName[INSTNAMESIZE];     // Name of the instrument
  WORD  wBagNdx;                      // Index to sfInstrumentBag
  WORD  wRefCount; // This field is used in EDIT ENGINE for reference counts

  //// For Windows 3.1 and Borland's huge pointers //// 
  #ifdef EMU_WINDOWS
  DWORD dwPad0;
  DWORD dwPad1;
  #endif
} sfInst;

// A flag in the sfSampleType field of the Sample Header data structure.
// This flag indicates that the sampled waveform it uses
// is expected to be on a WaveTable ROM, and is NOT in the file.
#define romSample 0x8000

// The other defined values for the sfSampleType field of the Sample Header
// data structure.
typedef enum {

  monoSample   = 1, // The sample is a MONO sample
  rightSample  = 2, // The sample is the RIGHT sample of a stereo pair 
                    // or the first in a group of linked samples
  leftSample   = 4, // The sample is the LEFT sample of a stereo pair
  linkedSample = 8  // The sample is another in a group of linked samples
                    // the last of which should point to the sample with
                    // rightSample set.

}SFSampleLink; 


const WORD smplHdrV2ExtensionSize    =  (3*sizeof(WORD)+sizeof(DWORD));
const WORD smplHdrV2Size             =  36 + smplHdrV2ExtensionSize;

//// Standard list used in Shdr (Sample header lists) ////
typedef struct sfSampleHdrTag
{

  CHAR          achSampleName[SAMPLENAMESIZE];
  DWORD         dwStart;          // Sample addresses
  DWORD         dwEnd;
  DWORD         dwStartloop;
  DWORD         dwEndloop;
  DWORD         dwSampleRate;     // In Hz, IE 44100, 22050, etc
  BYTE          byOriginalKey;    // MIDI Key, 0 to 127
  CHAR          chFineCorrection; // Tuning correction in cents
  WORD          wSampleLink;   // index to 'next' sample in chain
  WORD          sfSampleType;  // the kind (values: see enum SFSampleLink )

  BOOL          bSampleLoaded; // This field is used in RUNTIME ENGINE ONLY
                               // as a convenient flag to indicate loaded vs
                               // unloaded samples.

  WORD  wRefCount; // This field is used in EDIT ENGINE for reference counts 

} sfSampleHdr;


////////////////////////////////////////////////////
// Typedefs for working with segmented architecture
////////////////////////////////////////////////////

#if defined (EMU_WINDOWS) || defined (DOS_MEMORY)

typedef sfPresetHdr huge* SFPRESETHDRPTR;
typedef sfInst huge*      SFINSTPTR;
typedef sfBagNdx huge*    SFBAGNDXPTR;
typedef sfGenList huge*   SFGENLISTPTR;
typedef sfModList huge*   SFMODLISTPTR;
typedef sfSampleHdr huge* SFSAMPLEHDRPTR;

///////////////////////////////////////////////////////
// Typedefs for working with non segmented architecture
///////////////////////////////////////////////////////

#else

typedef sfPresetHdr* SFPRESETHDRPTR;
typedef sfInst*      SFINSTPTR;
typedef sfBagNdx*    SFBAGNDXPTR;
typedef sfGenList*   SFGENLISTPTR;
typedef sfModList*   SFMODLISTPTR;
typedef sfSampleHdr* SFSAMPLEHDRPTR;

#endif // Memory typedef dependencies


/////////////////////////////////
// DS for holding Sample Name  //
/////////////////////////////////

class HydraClass 
{
  public:
    //// Methods ////
    HydraClass(void);
    ~HydraClass(void);

    void ConstructHydra(void);
    void DestructHydra(void);

    void  ZapHydra(void);
    void  ResetDefault(void);

    const sfData* getDefault(void) { return &sfDefault; }

    void  SetVersion(WORD majorVersion, WORD minorVersion) 
        {HydraVersion.wMajor = majorVersion; 
         HydraVersion.wMinor = minorVersion;}
    void  GetVersion(WORD *majorVersion, WORD *minorVersion) 
        {*majorVersion = HydraVersion.wMajor;
         *minorVersion = HydraVersion.wMinor;}

    const CHAR *GetBankName(void) {return (const CHAR *)achBankName;}
    EMUSTAT     SetBankName(CHAR *theNewName);

    //// The public data ////
    
    // These 9 pointers point to the in-memory image of the SoundFont file data.
    // The only differences between this image of the SoundFont data 
    // and the data as it exists in the file itself are 
    // (1) The existence of 'reference count' fields in pInst and pSHdr
    // (2) The existence of 'sample loaded' boolean field in pSHdr
    // (3) SFBanks loaded on BYTE_INCOHERENT drivers have appropriate bytes 
    //     pre-swapped.

    SFPRESETHDRPTR pPHdr;
    SFBAGNDXPTR    pPBag;
    SFGENLISTPTR   pPGen;
    SFMODLISTPTR   pPMod;
    SFINSTPTR      pInst;
    SFBAGNDXPTR    pIBag;
    SFGENLISTPTR   pIGen;
    SFMODLISTPTR   pIMod;
    SFSAMPLEHDRPTR pSHdr;

    // This is an array used by the enabler and the edit engine to provide
    // convenient way to detect and prevent over-indexing the various arrays.
    WORD           awStructSize[SF_DATASTRUCTS]; // The count of each struct

    // This contains the default SoundFont generator vector, IE what each 
    // generator value should be if NO generator of a particular type exists 
    // within all of the appropriate key/velocity SPLITS of given INSTRUMENT.
    sfData sfDefault;

    CHAR      *achBankName; 
    sfVersion HydraVersion;
};

#endif // __HYDRA_H
