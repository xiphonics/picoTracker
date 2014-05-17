     /*********************************************************************
     
     sfenum.h
     
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
*  @(#)sfenum.h	1.1 12:06:32 3/15/95 12:06:36
*
* Filename: sfenum.h
*
* Description: Generator enumerations for SoundFont data structure  
*
* NOTE: All enumerations called unusedX should be considered RESERVED for
*       possible FUTURE use!
*******************************************************************************
*/
#ifndef __SFENUM_H
#define __SFENUM_H

/////////////////////////////
//       Includes          //
/////////////////////////////

/////////////////////////////
//       Defines           //
/////////////////////////////

/////////////////////////////
//     Enumerations        //
/////////////////////////////

typedef enum 
{
  //// Oscillator ////
  startAddrsOffset,             //  0 sample start fine offset
  endAddrsOffset,               //  1 sample end fine offset
  startloopAddrsOffset,         //  2 sample start loop fine offset
  endloopAddrsOffset,           //  3 sample end loop file offset
  startAddrsCoarseOffset,       //  4 sample start coarse offset 
  modLfoToPitch,                //  5 main fm: modLfo-> pitch
  vibLfoToPitch,                //  6 aux fm:  vibLfo-> pitch
  modEnvToPitch,                  //  7 pitch env: modEnv(aux)-> pitch

  //// Filter ////
  initialFilterFc,              //  8 initial filter cutoff
  initialFilterQ,               //  9 filter Q
  modLfoToFilterFc,               // 10 filter modulation: lfo1 -> filter cutoff
  modEnvToFilterFc,               // 11 filter env: modEnv(aux)-> filter cutoff

  //// Amplifier ////
  endAddrsCoarseOffset,         // 12 initial volume
  modLfoToVolume,                 // 13 tremolo: lfo1-> volume
  unused0,                      // 14 unused

  //// Effects ////
  chorusEffectsSend,            // 15 chorus
  reverbEffectsSend,            // 16 reverb
  pan,                          // 17 pan
  unused1,                      // 18 unused
  unused2,                      // 19 unused
  unused3,                      // 20 unused

  //// Modulation LFO ////
  delayModLfo,                    // 21 delay 
  freqModLfo,                     // 22 frequency

  //// Vibrato LFO ////
  delayVibLfo,                    // 23 delay 
  freqVibLfo,                     // 24 frequency

  //// Modulation Envelope ////
  delayModEnv,                    // 25 delay 
  attackModEnv,                   // 26 attack
  holdModEnv,                     // 27 hold
  decayModEnv,                    // 28 decay
  sustainModEnv,                  // 29 sustain
  releaseModEnv,                  // 30 release
  keynumToModEnvHold,             // 31 key scaling coefficient
  keynumToModEnvDecay,            // 32 key scaling coefficient

  //// Volume Envelope (ampl/vol) ////

  delayVolEnv,                    // 33 delay 
  attackVolEnv,                   // 34 attack
  holdVolEnv,                     // 35 hold
  decayVolEnv,                    // 36 decay
  sustainVolEnv,                  // 37 sustain
  releaseVolEnv,                  // 38 release
  keynumToVolEnvHold,             // 39 key scaling coefficient
  keynumToVolEnvDecay,            // 40 key scaling coefficient

  //// Preset ////
  instrument,                   // 41
  nop,                          // 42
  keyRange,                     // 43
  velRange,                     // 44
  startloopAddrsCoarseOffset,   // 45 
  keynum,                       // 46
  velocity,                     // 47
  initialAttenuation,           // 48
  keyTuning,                    // 49
  endloopAddrsCoarseOffset,     // 50

  coarseTune,                   // 51
  fineTune,                     // 52
  sampleId,                     // 53
  sampleModes,                  // 54
  unused4,                      // 55
  scaleTuning,                  // 56
  exclusiveClass,               // 57
       
  overridingRootKey,           // 58
  unused5,                     // 59
  endOper                      // 60

} SFGenerator;   

// To be defined at a later date
typedef enum 
{
  endMod
} ModulatorTypes;





#endif // __SFENUM_H
//////////////////////// End of SFENUM.H ////////////////////////////
