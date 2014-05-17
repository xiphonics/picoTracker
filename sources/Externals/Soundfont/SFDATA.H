     /*********************************************************************
     
     sfdata.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
#ifndef __SFDATA_H
#define __SFDATA_H


/******************************************************************************
*
*     Copyright (c) E-mu Systems, Inc. 1994. All rights Reserved.
*                             
*******************************************************************************
*/

/*****************************************************************************
*  @(#)sfdata.h	1.1 12:06:31 3/15/95 12:06:36
*
* Filename: sfdata.h
*
* Description: SoundFont Data Structure
*
*******************************************************************************
*/

/////////////////////////////
//       Includes          //
/////////////////////////////

#include "DATATYPE.H" 

/////////////////////////////
// SoundFont Data Structure//
/////////////////////////////

typedef enum sfSampleFlagsTag
{
  NOFLAG = 0,
  ZL = 1,
  ZR = 2,
  LINKED = 4,
  FIRST_LINK = 8
} sfSampleFlags;

typedef struct sfDataTag
{
  //// Oscillator ////
  DWORD dwStart;             //// sample start address 
  DWORD dwEnd;
  DWORD dwStartloop;         //// loop start address 
  DWORD dwEndloop;           //// loop end address 
  DWORD dwSampleRate; 
  SHORT shOrigKeyAndCorr;
  SHORT shSampleModes;
  SHORT shSampleLink;

  //// Pitch ////
  SHORT  shCoarseTune;
  SHORT  shFineTune;
  SHORT  shScaleTuning;
  SHORT  shModLfoToPitch;            //// main fm: modLfo-> pitch ////
  SHORT  shVibLfoToPitch;            //// aux fm:  vibLfo-> pitch ////
  SHORT  shModEnvToPitch;            //// pitch env: modEnv(aux)-> pitch ////

  //// Filter ////
  SHORT   shInitialFilterFc;        //// initial filter cutoff ////
  SHORT   shInitialFilterQ;         //// filter Q ////
  SHORT   shModLfoToFilterFc;         //// modLfo -> filter * cutoff ////
  SHORT   shModEnvToFilterFc;         //// mod env(aux)-> filter * cutoff ////

  //// Amplifier ////
  SHORT   shInstVol;                //// ////
  SHORT   shModLfoToVolume;           //// tremolo: modLfo-> volume ////

  //// Effects ////
  SHORT   shChorusEffectsSend;      //// chorus ////
  SHORT   shReverbEffectsSend;      //// reverb ////
  SHORT   shPanEffectsSend;         //// pan ////

  //// Modulation Low Frequency Oscillator ////
  SHORT   shDelayModLfo;              //// delay 
  SHORT   shFreqModLfo;               //// frequency ////

  //// Vibrato (Pitch only) Low Frequency Oscillator ////
  SHORT   shDelayVibLfo;              //// delay 
  SHORT   shFreqVibLfo;               //// frequency ////

  //// Modulation Envelope ////
  SHORT   shDelayModEnv;              //// delay 
  SHORT   shAttackModEnv;             //// attack ////
  SHORT   shHoldModEnv;               //// hold ////
  SHORT   shDecayModEnv;              //// decay ////
  SHORT   shSustainModEnv;            //// sustain ////
  SHORT   shReleaseModEnv;            //// release ////
  SHORT   shAutoHoldModEnv;
  SHORT   shAutoDecayModEnv;

  //// Attenuation (Volume only) Envelope ////
  SHORT   shDelayVolEnv;              //// delay 
  SHORT   shAttackVolEnv;             //// attack ////
  SHORT   shHoldVolEnv;               //// hold ////
  SHORT   shDecayVolEnv;              //// decay ////
  SHORT   shSustainVolEnv;            //// sustain ////
  SHORT   shReleaseVolEnv;            //// release ////
  SHORT   shAutoHoldVolEnv;
  SHORT   shAutoDecayVolEnv;

  //// Miscellaneous ////
  SHORT   shKeyExclusiveClass;

  //// Preserved for informational purposes ////
  SHORT   shKeynum;                 //// ////
  SHORT   shVelocity;               //// ////

  //// These parameters are processed from within navigator ////
  SHORT   shStartAddrsCoarseOffset;
  SHORT   shEndAddrsCoarseOffset;
  SHORT   shStartloopAddrsCoarseOffset;
  SHORT   shEndloopAddrsCoarseOffset;
  SHORT   shOverridingRootKey;

  //// Place holders, not used ////
  SHORT   shNOP;
  SHORT   shEndOper;
} sfData;




#endif // __SFDATA_H
//////////////////////// End of SFDATA.H ////////////////////////////
