     /*********************************************************************
     
     sfnav.h
     
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
*  @(#)sfnav.h	1.1 12:06:33 3/15/95 12:06:37
* 
* Filename: sfnav.h
*
* Description: The SoundFont Navigator
*
*******************************************************************************
*/

#ifndef __SFNAV_H
#define __SFNAV_H


/////////////////////////////
//        Includes         //
/////////////////////////////

#include "DATATYPE.H"
#include "SFDATA.H"
#include "HYDRA.H"

/////////////////////////////
//         Defines         //
/////////////////////////////

#define MAX_SAMPLES             4   // The max number of layers for a preset
#define SF_N_MEMBERS           58  // The number of SoundFont data items
#define MAX_SOUND_FONTS        10  // The max number of simultaneous SFB's

/////////////////////////////
//      Constants          //
/////////////////////////////

/////////////////////////////
//        Typedefs         //
/////////////////////////////

/////////////////////////////
//        Classes          //
/////////////////////////////

class SoundFontNavigator 
{
  public:
    SoundFontNavigator(void);
    ~SoundFontNavigator(void);

    void        Reset(void);
    void        Navigate(WORD wSFID, WORD wKey, WORD wVel);
    WORD        GetNOsc(void)  { return (wOsc);          }
    sfData*     GetSFPtr(void) { return (&(sfVector[0])); }
    void        GetHydraFont(HydraClass* pHydra);
    HydraClass* SetHydraFont(HydraClass* pHydra);
    WORD        GetSFNum(WORD wBank, BYTE byPatch, WORD* pwSFID);

  private:

    void        ProcessSampleLinks(void);
    void        AddSoundFonts(sfData *, sfData *);
    HydraClass    *phfNav;
    WORD          wOsc;                 // # of Osc's used for Preset
    sfData        sfPresetData;          // The preset global layer data
    sfData        sfInstData;            // The instrument global data
    sfData        sfCurrPreset;          // The current preset layer data
    sfData        sfVector[MAX_SAMPLES];  // Specific data for an oscillator
    WORD          shdrIndexLinks[MAX_SAMPLES];
    WORD          nextOscLinkCheck;
    BYTE          linkFound[MAX_SAMPLES];

};



#endif //  __SFNAV_H

