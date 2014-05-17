/*********************************************************************

enab.cpp

Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
PURPOSE.

*********************************************************************/
//-****************************************************************************
//
//                          Copyright (c) 1995
//               E-mu Systems Proprietary All rights Reserved.
//                             
//-****************************************************************************

#include <iostream>

#include "SFNAV.H"
#include "SFREADER.H"
#include "SFDETECT.H"
#include "ENAB.H"

using namespace::std ;
using namespace::soundfont ;

//-****************************************************************************
// @(#)enab.cpp	1.3 14:07:22 5/31/95 14:07:22
// 
// Description: 
//
//   This module provides the implementation of the SoundFont Enabler. 
// Users may load multiple banks simultaniously, up to the limit of 
// MAXLOADEDBANKS. 
// 
// 
// Visible Routines: 
//  
//   sfReadSFBFile()
//   sfUnloadSFBank()
//   sfGetSampHdrs()
//   sfGetPresetHdrs()
//   sfGetSMPLOffset() 
//   sfNav()
//   sfGetError()
//   sfClearError()
// 
// Local Routines:
//
//   None.
//
//-****************************************************************************

typedef struct { 
  
 CHAR       *bankFileName; 
 HydraClass *pHydra; 
 DWORD       sampleCkOffset;

}bankNodeType; 

static bankNodeType bankNodes[MAXLOADEDBANKS]; 

static           BOOL bankNodesInit    = FALSE; 
static           SHORT        lastBank              = -1; 
static              enaErrors omega        = enaSUCCESS; 
sfReader             reader; 
sfDetectErrors       detect;
RIFFClass              riff; 
SoundFontNavigator      navit;

HydraClass *GetHydraPtr(int bankID) {
	return bankNodes[bankID].pHydra; 
}

sfBankID sfReadSFBFile(char *bankFileName)
//-****************************************************************************
// 
// Implementation Notes: 
//
//   Use the local sfReader class obj. 'reader' to readup the given 
// bankFileName as a SoundFont Bank. Run the resulting pHydra through the
// SoundFont error detector. If it passes, place the pHydra into the 
// the first available free node in our bankNodes array, which we initialize
// if first time through.
//
//-***************************************************************************
{
WORD i; 

 if( ! bankNodesInit ) { 
    memset(bankNodes,0,MAXLOADEDBANKS * sizeof(bankNodeType));
    bankNodesInit = TRUE;   
 }

 for(i = 0; i < MAXLOADEDBANKS; i++ ) { 

   if(bankNodes[i].pHydra == NULL ) { // use first we find...

     HydraClass *pHF;

     // Note there is no WaveTable ID in this call because this is
     // only displaying numbers on a screen. 

     pHF = reader.ReadSFBFile( bankFileName); 

     // If a REAL synthesizer were used, the given synth would need
     // to read in the WaveTable ID (however it works) and pass it in
     // as such:
     //CHAR *waveID = readInSynthWaveTableIDSomehow();
     //pHF = reader.ReadSFBFile( bankFileName, waveID); 

     if(pHF == NULL)
     {
       #ifdef DEBUG_ENAB 
	cout<<"%Enab-ReadBank-E- error returned from reader - " 
	    <<reader.GetError() << endl; 
       #endif
       omega = enaLOADERROR;
       return -1; 
     }
    
/*     else if (detect.VerifySFBData(pHF, reader.GetAllSampleSize()) == FALSE)
     {
       #ifdef DEBUG_ENAB 
	cout<<"%Enab-VerifyBank-E- error returned from error detector - " 
	    << endl; 
       #endif
       omega = enaVERIFYERROR;
       return -1; 
     }
*/
     else
     {
        bankNodes[i].pHydra = pHF;
     }

     bankNodes[i].bankFileName = new CHAR[strlen(bankFileName)+1]; 
     strcpy(bankNodes[i].bankFileName , bankFileName); 

     riff.OpenRIFF(bankFileName);
     riff.FindCk("SMPL"); 
     bankNodes[i].sampleCkOffset = riff.RIFFTell();
     riff.RIFFClose(); 

     #ifdef DEBUG_ENAB
	      cout<< endl;
      cout<<"File Name     : " << bankFileName <<endl; 
      cout<<"Bank Name     : " << bankNodes[i].pHydra->GetBankName()<< endl
          <<"Enab ID       : " << i << endl; 
      cout<<"SMPL Offset   : " << bankNodes[i].sampleCkOffset << endl; 
     #endif
     return i; 

   }
 }  
omega = enaEBANKTABLEFULL;  
return -1; 

}// end sfREadSFBFile;

SHORT sfUnloadSFBank( sfBankID whichBank )
//-****************************************************************************
// 
// Implementation Notes: 
// 
//    Unload the bank found at whichBank. Don't forget to delete the pHydra
// class. Set bankID to -1 for reuse.
//
//-***************************************************************************
{

   if(( whichBank < 0 ) || ( whichBank >= MAXLOADEDBANKS)) {
     omega = enaENOSUCHBANK;
     return -1; 
   }
   if( bankNodes[whichBank].pHydra != NULL ) { // we have it.
       cout<<"unloading bankID " << whichBank << "pHydra val: "
           << bankNodes[whichBank].pHydra << endl; 

       delete [] bankNodes[whichBank].bankFileName; 
       delete bankNodes[whichBank].pHydra; 
       memset(&bankNodes[whichBank],0,sizeof(bankNodeType));
   }
   else { 
     omega = enaWBANKNOTLOADED; 
     return  SUCCESS; 
   }
   return SUCCESS; 

}// end sfUnloadSFBank


SFSAMPLEHDRPTR  &sfGetSampHdrs( sfBankID fromBank, WORD  *cnt )
//-****************************************************************************
// 
// Implementation Notes: 
//
//   Just return the address of the samples hdrs array of the currently loaded
// bank.  
//
// Note: These are the keys to the car, drive safely.
//
//-***************************************************************************
{
/*  if ((fromBank >= MAXLOADEDBANKS) || ( fromBank < 0 )) { 
    omega = enaENOSUCHBANK; 
    return NULL; 
  }
  if(bankNodes[fromBank].pHydra == NULL) { 
    omega = enaWBANKNOTLOADED; 
    return NULL; 
  } */
  *cnt = bankNodes[fromBank].pHydra->awStructSize[sampHdr] -1; 
  return bankNodes[fromBank].pHydra->pSHdr; 

}// end sfGetSampHdrs; 


SFPRESETHDRPTR sfGetPresetHdrs( sfBankID fromBank, WORD *cnt )
//-****************************************************************************
// 
// Implementation Notes: 
//   
//   Just return the preset hdr's address of the hydra's preset hdr array.
// Note: These are the keys to the car, drive safely.
//
//-***************************************************************************
{
  if((fromBank >= MAXLOADEDBANKS) || ( fromBank < 0 )) { 
    omega = enaENOSUCHBANK; 
    return 0;  // never a valid sample chunk offset. 
  }
  if(bankNodes[fromBank].pHydra == NULL) { 
    omega = enaWBANKNOTLOADED; 
    return 0; 
  }
  *cnt = bankNodes[fromBank].pHydra->awStructSize[prstHdr] -1; 
  return bankNodes[fromBank].pHydra->pPHdr; 

}// end sfGetPresetHdrs; 


DWORD sfGetSMPLOffset( sfBankID fromBank )
//-****************************************************************************
// 
// Implementation Notes: 
//
//-***************************************************************************
{
  if ((fromBank >=  MAXLOADEDBANKS) || ( fromBank < 0 )) { 
    omega = enaENOSUCHBANK; 
    return 0;  // never a valid sample chunk offset. 
  }
  if ( bankNodes[fromBank].pHydra != NULL ) { 
    return bankNodes[fromBank].sampleCkOffset; 
  } 
  else { 
    omega = enaWBANKNOTLOADED; 
    return 0; 
  }
}// end sfGenSMPLOffset; 


sfData* sfNav(sfBankID sfBank,
              WORD     MidiBank,
              WORD     MidiPreset, 
	      WORD     MidiNote, 
	      WORD     KeyVelo, 
	      WORD     *cnt )
//-****************************************************************************
// 
// Implementation Notes: 
//
//   A _simple_ note on event, with built-in (read slow) 'bank select' and 
// 'patch change' type functionality. It passes back the SoundFont parameters
// gathered during the navigation of the note_on event. The size of the 
// vector of sfDatas is returned in cnt. 
//
//   The return value is the address of a private data member of the SoundFont
// Navigator class, or if you like, a static memory area that is reused from 
// call to call. 
//
//-***************************************************************************
{
WORD        oscCount; 
sfData      *sfVec; 
WORD        sfPrstIndex; 
WORD        stat ;

  if(( sfBank < 0 ) || ( sfBank >= MAXLOADEDBANKS)) { 
    omega = enaENOSUCHBANK; 
    *cnt  = 0; 
    return NULL; 
  }
  if( bankNodes[sfBank].pHydra == NULL ) { 
     omega = enaWBANKNOTLOADED; 
     return NULL;
  } 

  if( lastBank != sfBank ) { 
    lastBank = sfBank; 
    navit.SetHydraFont( bankNodes[sfBank].pHydra);   // would be bank select
  }

  // Speed improvement: check if Midi Bank or MidiPreset has changed...
  // if not, use the old (if there was one) value of sfPretIndex. If so, you 
  // must go obtain the new sfBankIndex via GetSFNum() (ie patch change)

  stat  = navit.GetSFNum( MidiBank, BYTE(MidiPreset), &sfPrstIndex);  // would be 
                                                                // patch change
  if( stat != SUCCESS) {  // could be a SF_INVALIDBANK if hydra is null or 
    *cnt = 0;            // SF_PRESETNOTFOUND if the 2-tuple not preset in 
                        // sfBank...Thats all the errors defined for GetSFNum()
    if(stat == SF_PRESETNOTFOUND) { 
      omega = enaENOSUCHPRESET; 
    }          
    else if(stat == SF_INVALIDBANK) { 
      omega = enaWBANKNOTLOADED; 
    }
    return NULL; 
  } // anything else must be some kind of internal error, 
 
  navit.Navigate( sfPrstIndex, MidiNote, KeyVelo );   // _the_ note on event
  // 
  // Now the sfVector private data member of the navit object holds some
  // number of sfdata objects between 1 and 4. This number represents the 
  // number of osillators that are required for this note. 
  // 
  oscCount = navit.GetNOsc();
  sfVec    = navit.GetSFPtr();
  *cnt     = oscCount; 

  return sfVec; 

}// end sfNav



void sfDumpBanksInfo()
{
	
  for(WORD i = 0; i < MAXLOADEDBANKS; i++ ) { 
    if(bankNodes[i].pHydra != NULL ) { 
         cout<< endl;
         cout<<"  File Name     : " <<bankNodes[i].bankFileName <<endl; 
         cout<<"  Bank Name     : " <<bankNodes[i].pHydra->GetBankName()<< endl
             <<"  Enab ID       : " <<i << endl; 
         cout<<"  SMPL Offset   : " <<bankNodes[i].sampleCkOffset <<endl<<endl;
    }
  }
  return; 
} // end sfDumpBanksInfo; 


enaErrors sfGetError(void)
{
  return omega; 
}// end sfGetError

void sfClearError(void)
{
  omega = enaSUCCESS; 
}


