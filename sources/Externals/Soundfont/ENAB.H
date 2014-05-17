#ifndef _ENAB_SF_H_
#define _ENAB_SF_H_

/*********************************************************************
     
     enab.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
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

#include "DATATYPE.H"
#include "SFDATA.H" 
#include "HYDRA.H"

//-****************************************************************************
// @(#)enab.h	1.2 13:56:10 5/31/95 13:56:10 
//
// Description: 
//  
//   This module defines utility routines to load, navigate, and obtain
// various pieces of information from, SoundFont banks. Collectively known as
// the "enabler", these routines can be used to prototype SoundFont playback
// implementations. 
// 
//   Functionality exists to load the articulation portion (preset data) of 
// a SoundFont bank into main system memory, and to navigate the data as 
// a MIDI note on event would, and return the SoundFont parameters gathered 
// for that particular event. The units are SoundFont units and would be 
// translated into units digestable by the  target sound engine by the client.
// That is, once navigated, the rest is up to the client caller. 
// 
//   Sampled wave form data is left in the src file. The client must write
// routines to gather the sample data, and write it someplace their target
// sound engine(s) can reference it. They must then update the in memory 
// articulation data to reflect where the data has gone. Routines exist in 
// this module to find the location within the src file of any particular
// "sample", and how big it is. Armed with this knowledge the client can
// write the routines to download all of a banks sample data in short order.
// For instance, a simple method might be to start at the beginning an write
// all sample data to on board RAM of a sound board, and add a constant offset
// to all the sample address in the banks sampleHdrs. By definition, the 
// sampleHdrs are 'normalized' to the src file, zero'ed to the SampleCkOffset.
// (ie, the first Sample has a dwStart value of 0 (zero)). Functionality 
// exists in this module to obtain _the_ sample Hdrs of the articulation data, 
// as well as obtaining the SampleCkOffset from the src File. Once you
// know where the sample chunk starts, and have the i'th sample hdr, a quick
// fseek() will take you to the first sample element of the i'th sampled wave
// form data. Next, once you write the sampled wave form data to its new home
// you know the new 'real' address, and having the sampleHdr in front of you, 
// you can update the hdr. During the navigation of the Note On Event, you 
// are returned this new address in the SoundFont vector. See the routines
// sfGetSampleHDdrs() and sfGetSampleCkOffset prologues for additional details.
// 
// NOTE: 
//   Most functions return an indication of error, but place the actual 
// error value in a private object. Use sfGetError() to retrieve if desired.
// sfClearError() can be used to clear the error prior to a new call. 
// 
// Routines: 
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
// 
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May '95      Initial Creation, Enabler
//-***************************************************************************

// As you may load multiple SoundFont Banks at a time, this define bounds
// the extent of the internal implementation, (array) of info nodes, one per
// loaded banks. Need more than the default? Change this and you got 'em.

# define MAXLOADEDBANKS 3

// This is the type of a sfBankID, -1 is an error condition.
typedef SHORT sfBankID; 

typedef enum { 

  enaSUCCESS,
  enaLOADERROR, 
  enaENOSUCHBANK, 
  enaENOSUCHPRESET, 
  enaEBANKTABLEFULL,
  enaWBANKNOTLOADED,
  enaEBANKALREADYLOADED,
  enaVERIFYERROR

}enaErrors;
  

sfBankID  sfReadSFBFile(char *bankFileName); 
//-****************************************************************************
// 
// Description: 
//
//   Load the given bank denoted by 'bankFileName'. A bank id of -1 is never
// valid, and indicates an error during load. Unique bank ids are returned
// for each bank concurrently loaded. There can never be more than
// MAXLOADEDBANKS loaded at once.
//
// Return Values: 
// 
//    0 <= X < MAXLOADEDBANKS   - SUCCESS bank loaded.
//    -1                        - Error, bank not loaded.
//
//   ETOOMANYBANKS  - MAXLOADEDBANKS has been reached
//   ELOADERROR     - Source file not found, protection violation, bad format
//                    etc. 
//-****************************************************************************


SHORT sfUnloadSFBank( sfBankID whichBank ); 
//-****************************************************************************
// 
// Description: 
// 
//   Unload the (assumed) loaded SoundFont Bank.
//
// Return Values: 
//
//   enaSUCCESS 
//   -1 on error 
//
// OmegaValues: 
//
//   enaENOSUCHBANK     - The given sfBankID was not valid, (neg or > MAXLOADED
//   enaWBANKNOLOADED   - The sfBankID was not currently loaded.
//
//-****************************************************************************


SFSAMPLEHDRPTR  &sfGetSampHdrs( sfBankID fromBank, WORD  *cnt ); 
//-****************************************************************************
// 
// Description: 
//  
//   This routine will return the sampleHdrs (digital wave form descriptors)
// from the previously loaded SoundFont bank with ID 'fromBank'. The sample
// Hdrs take the form of an array of size 'cnt`. The formal cnt is 
// pre-allocated by the caller  and written by this routine as the 
// extent of the vector of hdrs returned as the routine's "function result". 
// A sample hdr has the following form: (see hydra.h for details):
//
//         typedef struct sfSampleHdrTag
//          {
//            CHAR          achSampleName[SAMPLENAMESIZE]; // size 20
//            DWORD         dwStart;                      // Sample address
//            DWORD         dwEnd;                       
//            DWORD         dwStartloop;                
//            DWORD         dwEndloop;              
//            ...
//
//           } sfSampleHdr;
//
// *Note: The address of these hdrs are given in units of Samples (i.e., 16 bit
//    WORDs) such that ((dwEnd - dwStart)+1) * 2) yields sample size in bytes.
//    (Useful when reading the sample data from the src file, given 
//    sampleOffset returned by sfGetSMPLOffset() below...)
//
// *Warning: 
//
//    The formal return value is a typedef'ed pointer to the SoundFont's 
// in memory articulation data called the SampleHdrs array. An alias'ed pointer
// results, these are the keys to the car, _don't_ _delete_ it. Drive safely.
// Update these address values, thus "normalizing" the address to the sound
// memory. (where ever that might be). 
//
// Return Values: 
//
//   SUCCESS = non null "function" result, and formal cnt > 0; 
//   NULL on error, 
//
// OmegaValues: 
//
//   enaENOSUCHBANK     - fromBank >= MAXLOADEDBANKS
//   enaWBANKNOTLOADED  - fromBank in range  0 <= fromBAnk < MAXLOADEDBANK
//                        but no bank currently loaded in that slot.
// 
//-****************************************************************************


DWORD sfGetSMPLOffset( sfBankID fromBank ); 
//-****************************************************************************
// 
// Description: 
//
//    Returns the offset to the beginning of the sampleData 'chunk' in 
// 'fromBank's' src file. Units are in _bytes_, such that an fseek(f(x)) from
// SEEK_SET, followed by an fseek to some (samps[i].dwStart*2), from SEEK_CUR,
// (see sfGetSampHdrs()) will position the current file pointer to the first
// sample in the wave form data for the i'th sample. Each Src bank loaded will
// (likely) have a different sample chunk offset value. Once read and placed 
// in memory by the client, the sample address can be updated to reflect 
// their new location. As a result of sfNavigate(), the SoundFont vector(s)
// will also contain these sample address values. 
//
// NOTE: Samples values are Little Endian in nature. If the data is read out 
//       using fread, software on Big Endian systems will need to SWAP THE 
//       BYTES on ALL samples BEFORE using them in their synthesizer!
// 
// Return Values: 
//
//   N where N is non-zero.  
//   0 (zero) on error.
//
// OmegaValues: 
//
//   enaENOSUCHBANK     - ( 0 > fromBank || fromBank >= MAXLOADEDBANKS )
//   enaWBANKNOTLOADED  - fromBank in range, but not pointing at a currently 
//                        loaded bank. 
//-****************************************************************************

SFPRESETHDRPTR sfGetPresetHdrs( sfBankID fromBank, WORD *cnt ); 
//-****************************************************************************
// 
// Description: 
// 
//   This routine returns the preset hdrs, (SoundFont Object descriptors)
// from the currently loaded SoundFont bank with ID 'fromBank'. A Preset's
// hdr contains the preset name, MIDI preset number, and MIDI Bank number, 
// among other things). The hdrs are returned in an array of size cnt, which
// must be a pre-allocated WORD buffer to receive the actual size of the 
// returned array. The preset hdr data struct is reproduced below for 
// convenience, see hydra.h for the complete definition.
//
//        typedef struct { 
//          
//             CHAR   achPresetName[PRESETNAMESIZE]; // Name of preset  size 20
//             WORD   wPresetNum;                   // The MIDI preset number.
//             WORD   wPresetBank;                 // The MIDI bank number.
//             ...
//
//          } sfPresetHdr;
//
// Return Values: 
//
//   Address of preset headers (descriptors) on SUCCESS, cnt is the extent.
//   NULL on error;
//
// OmegaValues: 
//
//   enaENOSUCHBANK     : fromBank < 0 || fromBank >= MAXLOADEDBANKS
//   enaWBANKNOTLOADED  : fromBAnk in range, yet not pointing at a currently
//                        loaded bank, first use sfReadSFBFile().
//-****************************************************************************
HydraClass *GetHydraPtr(int bankID) ;

sfData*  sfNav(sfBankID sfBank,
               WORD     MidiBank, 
	       WORD     MidiPreset, 
	       WORD     MidiNote, 
	       WORD     KeyVelo, 
	       WORD     *cnt );
//-****************************************************************************
// 
// Description: 
//
//   Given an enabler bankID, MIDI bank number, MIDI preset number,
// MIDI note number, and a key velocity, this routine will navigate the 
// 'sfPreset' in 'sfBank' and return a vector of sfData elements. 
//
//   The vector has an extent of 'cnt' elements. The 'cnt' 
// formal is a user allocated buffer of size sizeof(WORD) that is available 
// to be written into by this routine. There will be 'cnt' elements of type
// sfData in the returned array. The sfData type is found in sfdata.h and 
// due to its size, is not reproduced here. The values in the sfdata vector
// are in SoundFont units and must be translated by the caller into units
// appropriate to the target sound engine. 
//
// *Note:  SoundFont banks can contain multiple MIDI banks or snipits of 
//         different MidiBanks.  Don't confuse the sfBank
//         and MIDIBank formals. If you ask for a MIDIBank not found in 
//         sfBank, You get a ENOSUCHPRESET since the midiBank/midiPreset 
//         two-tuple is not found in any of sfBank's preset Hdrs. See the 
//         routine sfGetPresetHdrs() to pre-determine if your MIDI bank/MIDI
//         preset 'patch' is present in the given SoundFont Bank. 
// 
// Return Values: 
//
//   Address of a static memory area of sfData structures.  'cnt' returns 
//           the current number of valid elements in the area. The number of 
//           elements corresponds to the number of voices, or oscillators 
//           in the definition of the preset given the particular note on 
//           event.
//   NULL on error.
// 
//   OmegaValues: 
//
//   enaENOSUCHBANK     : fromBank < 0 || fromBank >= MAXLOADEDBANKS
//   enaWBANKNOTLOADED  : fromBAnk in range, yet not pointing at a currently
//                        loaded bank, first use sfReadSFBFile().
//-****************************************************************************

void sfDumpBanksInfo();
//-****************************************************************************
// 
// Description: 
//
//   This routine will dump (to stdout) the internal bank descriptors used by 
// the 'enabler' to track the different banks that may be loaded at any given 
// time. Which brings up the question, "can I load the same bank twice?" 
// You bet. "Does it make sense?" Dunno. Depends. 
// 
//   The info for each currently loaded bank: 
//
//      FileName of the banks src file
//      SoundFont bank name
//      Enabler ID assigned to the bank
//      Sample Ck Offset value (in bytes) of FileName
// 
// *Note This routine exists only for test support. The enabdrive.c driver
// uses it, but nothing else will care if you remove it.
// 
// Return Values: 
//   None.
//-****************************************************************************


enaErrors sfGetError(void); 
//-****************************************************************************
// 
// Description: 
//
//   While not extensive, there are a few error values defined in this module.
// In general, if routines return a -1, (or NULL for those that return PTRs)
// some sort of error has been encountered, this routine will return the last
// error value. You may clear the error prior to a call using the 
// sfClearError() routine.
//
// Return Values: 
//   The last enaErrors value, enaSUCCESS if no error.
//-****************************************************************************

void sfClearError(void); 
//-****************************************************************************
// 
// Description: 
//
//   Use this routine to 'clear' an error condition if desired. 
//
// Return Values: 
//   None.
//-****************************************************************************

#endif
