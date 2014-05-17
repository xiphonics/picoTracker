     /*********************************************************************
     
     hydra.cpp
     
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

/////////////////////////////
//        Includes         //
/////////////////////////////

#include "HYDRA.H"
#include "WIN_MEM.H"
#include <string.h>

using namespace::soundfont ;

//*****************************************************************************
// @(#)hydra.cpp	1.2 09:40:33 3/21/95 09:40:33 
// Description: 
//     This file defines the datatype HydraFont. The datatype HydraFont
// is the base level in memory datatype used to describe a single
// SoundFont bank. 
// 
//*****************************************************************************

//////////////////////////
// The Constructor
//////////////////////////
HydraClass::HydraClass(void)
{
  ConstructHydra();
} // end constructor


//////////////////////////
// The Destructor
//////////////////////////
HydraClass::~HydraClass(void)
{
  DestructHydra();
}


////////////////////////////
// The callable Constructor
////////////////////////////
void HydraClass::ConstructHydra(void)
{
  pPHdr = NULL;
  achBankName = NULL;
  HydraVersion.wMajor = HydraVersion.wMinor = 0;
  ZapHydra();
  ResetDefault();
}


////////////////////////////
// The callable Destructor
////////////////////////////
void HydraClass::DestructHydra(void)
{
  ZapHydra();
}

EMUSTAT HydraClass::SetBankName(CHAR *theNewBankName)
{
  if (achBankName != NULL)
  {
    delete [] achBankName;
    achBankName = NULL;
  }

  if ((achBankName = new CHAR[strlen(theNewBankName)+1]) == NULL)
    return SF_MEMORYERROR;

  memset(achBankName, 0, strlen(theNewBankName));
  strcpy(achBankName, theNewBankName);
  return SUCCESS;
}

///////////////////////////////////////////
// Set all of the default SoundFont values
///////////////////////////////////////////
void  HydraClass::ResetDefault(void)
{

 // Sample
 sfDefault.dwStart                  =      0;
 sfDefault.dwEnd                    =      0;
 sfDefault.dwStartloop              =      0;
 sfDefault.dwEndloop                =      0;
 sfDefault.dwSampleRate             =      0;
 sfDefault.shOrigKeyAndCorr         = 0x3C00;
 sfDefault.shSampleModes            =      0;

 // Pitch
 sfDefault.shScaleTuning               =    100;  // 100 semitones/keynum
 sfDefault.shCoarseTune                =      0;
 sfDefault.shFineTune                  =      0;
 sfDefault.shModLfoToPitch             =      0;
 sfDefault.shVibLfoToPitch             =      0;
 sfDefault.shModEnvToPitch             =      0;

 // Filter

 sfDefault.shInitialFilterFc         =  13500;    // > 20 kHz
 sfDefault.shInitialFilterQ          =      0;     
 sfDefault.shModLfoToFilterFc        =      0;   
 sfDefault.shModEnvToFilterFc        =      0;  

 // Amplifier...

 sfDefault.shInstVol                 =      0;   // no attenuation
 sfDefault.shModLfoToVolume          =      0; 

 // Effects...

 sfDefault.shChorusEffectsSend       =      0;  
 sfDefault.shReverbEffectsSend       =      0; 
 sfDefault.shPanEffectsSend          =      0;

 // Modulation Low Frequency Oscillator

 sfDefault.shDelayModLfo               = -12000;  // < 1 ms
 sfDefault.shFreqModLfo                =      0;

 // Vibrato (Pitch only) Low Frequency Oscillator

 sfDefault.shDelayVibLfo               = -12000;  // < 1 ms
 sfDefault.shFreqVibLfo                =      0;  

 // Modulation Envelope

 sfDefault.shDelayModEnv               = -12000;  // < 1 ms
 sfDefault.shAttackModEnv              = -12000;  // < 1 ms
 sfDefault.shHoldModEnv                = -12000;  // < 1 ms
 sfDefault.shDecayModEnv               = -12000;  // < 1 ms
 sfDefault.shSustainModEnv             =      0;
 sfDefault.shReleaseModEnv             = -12000;  // < 1 ms
 sfDefault.shAutoHoldModEnv            =      0;
 sfDefault.shAutoDecayModEnv           =      0;

 // Attenuation (Volume only) Envelope

 sfDefault.shDelayVolEnv               = -12000;  // < 1 ms
 sfDefault.shAttackVolEnv              = -12000;  // < 1 ms
 sfDefault.shHoldVolEnv                = -12000;  // < 1 ms
 sfDefault.shDecayVolEnv               = -12000;  // < 1 ms
 sfDefault.shSustainVolEnv             =      0;
 sfDefault.shReleaseVolEnv             = -12000;  // < 1 ms
 sfDefault.shAutoHoldVolEnv            =      0;
 sfDefault.shAutoDecayVolEnv           =      0;

 // Miscellaneous

 sfDefault.shKeyExclusiveClass       =      0;

 // Preserved for informational purposes

 sfDefault.shKeynum                  =      0;
 sfDefault.shVelocity                =      0;
 sfDefault.shStartAddrsCoarseOffset      =      0;
 sfDefault.shEndAddrsCoarseOffset        =      0;
 sfDefault.shStartloopAddrsCoarseOffset  =      0;
 sfDefault.shEndloopAddrsCoarseOffset    =      0;
 sfDefault.shOverridingRootKey           =     -1;

 // Place holders, not used

 sfDefault.shNOP                     =      0;
 sfDefault.shEndOper                 =      0;

}


///////////////////////////////////////
// Deallocate the Hydra's pointers
///////////////////////////////////////
void  HydraClass::ZapHydra(void)
{
  WORD  wNdx;

  if (achBankName != NULL)
  {
    delete [] achBankName;
    achBankName = NULL;
  }
  
  HydraVersion.wMajor = HydraVersion.wMinor = 0;
  if (pPHdr != NULL)
  {
    if (pPHdr != NULL) Dealloc((VOIDPTR)pPHdr);
    if (pPBag != NULL) Dealloc((VOIDPTR)pPBag);
    if (pPGen != NULL) Dealloc((VOIDPTR)pPGen);
    if (pPMod != NULL) Dealloc((VOIDPTR)pPMod);
    if (pInst != NULL) Dealloc((VOIDPTR)pInst);
    if (pIBag != NULL) Dealloc((VOIDPTR)pIBag);
    if (pIGen != NULL) Dealloc((VOIDPTR)pIGen);
    if (pIMod != NULL) Dealloc((VOIDPTR)pIMod);
    if (pSHdr != NULL) Dealloc((VOIDPTR)pSHdr);
  }

  pPHdr        = NULL;
  pPBag        = NULL;
  pPGen        = NULL;
  pPMod        = NULL;
  pInst        = NULL;
  pIBag        = NULL;
  pIGen        = NULL;
  pIMod        = NULL;
  pSHdr        = NULL;

  for (wNdx = 0; wNdx < SF_DATASTRUCTS; wNdx++)
  {
    awStructSize[wNdx] = 0;
  }
}

///////////////////////// End of HYDRA.CPP //////////////////////////
