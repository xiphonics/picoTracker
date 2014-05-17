     /*********************************************************************
     
     sflookup.cpp
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
//-*********************************************************************
// 
//     Copyright (C) 1995 E-mu Systems, Inc. All Rights Reserved.
// 
//-*********************************************************************

//-*********************************************************************
// sflookup.cpp: A lookup which gives a SFENUM to SFVECTOR translation
//               by using simple indexing. 
// 
// NOTE: This is its own module in case other objects need the same
//       lookup table. In the SoundFont enabler, this is not the 
//       case.
//
// WARINING: Make sure your SFENUM lies within the scope of this 
//           lookup table, otherwise you may find yourself indexing
//           into other memory space!
//-*********************************************************************
#include "DATATYPE.H" 
#include "SFDATA.H"
#include "SFENUM.H"
#include "SFLOOKUP.H"

#ifndef NULL
#define NULL 0
#endif

SFLOOKUP *sfLookup;
SHORT sfLookupCount = 0;

//-*********************************************************************
// Return a pointer to the lookup table
//-*********************************************************************
const SFLOOKUP *GetLookup(void)
{
  if (sfLookupCount > 0) 
    return (const SFLOOKUP *)sfLookup;
  return (const SFLOOKUP *) NULL;
}

//-*********************************************************************
// Free the lookup table from heap if no other module is using it.
//-*********************************************************************
void ClearLookup(void)
{
  if ((sfLookupCount == 1) && (sfLookup != NULL))
  {
    delete [] sfLookup;
  }

  if (sfLookupCount)
    sfLookupCount --;
}

//-*********************************************************************
// Allocate the lookup table from heap if no other module has done so.
// Otherwise, bump the internal reference count by one.
//-*********************************************************************
void InitLookup(void)
{
  if (sfLookupCount > 0) 
  {
    sfLookupCount ++;
    return;
  }
  
  if ((sfLookup = new SFLOOKUP[endOper+5]) == NULL)
    return;
  
  WORD count = 0;
  while (count < endOper+5)
  {
    sfLookup[count] = MEMBEROFFSET(sfData, shNOP);
    count++;
  }
    
  sfLookup[0] = MEMBEROFFSET(sfData, dwStart);            /* startAddrs */
  sfLookup[1] = MEMBEROFFSET(sfData, dwEnd);              // endloopAddrs
  sfLookup[2] = MEMBEROFFSET(sfData, dwStartloop);        // endAddrs
  sfLookup[3] = MEMBEROFFSET(sfData, dwEndloop);          // startloopAddrs
  sfLookup[4] = MEMBEROFFSET(sfData, shStartAddrsCoarseOffset);      
  sfLookup[5] = MEMBEROFFSET(sfData, shModLfoToPitch);       // lfo1ToPitch
  sfLookup[6] = MEMBEROFFSET(sfData, shVibLfoToPitch);       // lfo2ToPitch
  sfLookup[7] = MEMBEROFFSET(sfData, shModEnvToPitch);       // env1ToPitch

  //// Filter ////
  sfLookup[8] = MEMBEROFFSET(sfData, shInitialFilterFc);   // shnitialFilterFc
  sfLookup[9] = MEMBEROFFSET(sfData, shInitialFilterQ);    // shnitialFilterQ
  sfLookup[10] = MEMBEROFFSET(sfData, shModLfoToFilterFc);    // lfo1ToFilterFc
  sfLookup[11] = MEMBEROFFSET(sfData, shModEnvToFilterFc);    // env1ToFilterFc

  //// Amplifier ////
  sfLookup[12] = MEMBEROFFSET(sfData, shEndAddrsCoarseOffset);
  sfLookup[13] = MEMBEROFFSET(sfData, shModLfoToVolume);      // lfo1ToVolume

  //// Effects ////
  sfLookup[15] = MEMBEROFFSET(sfData, shChorusEffectsSend); // chorusEffectsSend
  sfLookup[16] = MEMBEROFFSET(sfData, shReverbEffectsSend); // reverbEffectsSend
  sfLookup[17] = MEMBEROFFSET(sfData, shPanEffectsSend);    // panEffectsSend

  //// Main LFO1 ////
  sfLookup[21] = MEMBEROFFSET(sfData, shDelayModLfo);         // delayModLfo
  sfLookup[22] = MEMBEROFFSET(sfData, shFreqModLfo);          // freqModLfo

  //// Aux VolEnv ////
  sfLookup[23] = MEMBEROFFSET(sfData, shDelayVibLfo);         // delayVibLfo
  sfLookup[24] = MEMBEROFFSET(sfData, shFreqVibLfo);         // freqVibLfo

  //// Envelope1 ////
  sfLookup[25] = MEMBEROFFSET(sfData, shDelayModEnv);         // delayModEnv
  sfLookup[26] = MEMBEROFFSET(sfData, shAttackModEnv);        // attackModEnv
  sfLookup[27] = MEMBEROFFSET(sfData, shHoldModEnv);          // holdModEnv
  sfLookup[28] = MEMBEROFFSET(sfData, shDecayModEnv);         // decayModEnv
  sfLookup[29] = MEMBEROFFSET(sfData, shSustainModEnv);       // sustainModEnv
  sfLookup[30] = MEMBEROFFSET(sfData, shReleaseModEnv);       // releaseModEnv
  sfLookup[31] = MEMBEROFFSET(sfData, shAutoHoldModEnv);      // autoHoldModEnv
  sfLookup[32] = MEMBEROFFSET(sfData, shAutoDecayModEnv);     // autoDecayModEnv

  //// Envelope2 ////
  sfLookup[33] = MEMBEROFFSET(sfData, shDelayVolEnv);         // delayVolEnv
  sfLookup[34] = MEMBEROFFSET(sfData, shAttackVolEnv);        // attackVolEnv
  sfLookup[35] = MEMBEROFFSET(sfData, shHoldVolEnv);          // holdVolEnv
  sfLookup[36] = MEMBEROFFSET(sfData, shDecayVolEnv);         // decayVolEnv
  sfLookup[37] = MEMBEROFFSET(sfData, shSustainVolEnv);       // sustainVolEnv
  sfLookup[38] = MEMBEROFFSET(sfData, shReleaseVolEnv);       // releaseVolEnv
  sfLookup[39] = MEMBEROFFSET(sfData, shAutoHoldVolEnv);      // autoHoldVolEnv
  sfLookup[40] = MEMBEROFFSET(sfData, shAutoDecayVolEnv);     // autoDecayVolEnv

  //// Preset Data ////
  sfLookup[45] = MEMBEROFFSET(sfData, shStartloopAddrsCoarseOffset);
  sfLookup[46] = MEMBEROFFSET(sfData, shKeynum);
  sfLookup[47] = MEMBEROFFSET(sfData, shVelocity);
  sfLookup[48] = MEMBEROFFSET(sfData, shInstVol);
  sfLookup[50] = MEMBEROFFSET(sfData, shEndloopAddrsCoarseOffset);
  sfLookup[51] = MEMBEROFFSET(sfData, shCoarseTune);
  sfLookup[52] = MEMBEROFFSET(sfData, shFineTune);
  sfLookup[54] = MEMBEROFFSET(sfData, shSampleModes);
  sfLookup[55] = MEMBEROFFSET(sfData, shOrigKeyAndCorr);
  sfLookup[56] = MEMBEROFFSET(sfData, shScaleTuning);
  sfLookup[57] = MEMBEROFFSET(sfData, shKeyExclusiveClass);
  sfLookup[58] = MEMBEROFFSET(sfData, shOverridingRootKey);
 
  sfLookupCount = 1;
  
};
