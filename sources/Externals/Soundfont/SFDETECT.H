     /*********************************************************************
     
     sfdetect.h
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
/*============================================================================
*
*                          Copyright (c) 1995
*               E-mu Systems Proprietary All rights Reserved.
*                             
*============================================================================
*/

/*============================================================================
* @(#)sfdetect.h	1.2 13:56:04 3/22/95 13:56:06
*                          
*  FILE :   sfdetect.h
*
*  Description:  An object to detect and report errors from within the
*                Memory Image of the SoundFont data structure. 
*
*  NOTE: Currently only detects errors and does very minor repairs. The
*        error reporting mechanism will be introduced to this object along
*        with the forthcoming SoundFont validator.
*
*============================================================================
*/

#ifndef __SFDETECT_H
#define __SFDETECT_H

#include "DATATYPE.H"
#include "HYDRA.H"

class sfDetectErrors
{
  public:
    sfDetectErrors() {;}
    ~sfDetectErrors() {;}

    BOOL VerifySFBData(HydraClass *pHF, DWORD sampleRAMSizeInBytes);
    BOOL VerifyPDTAIndices(HydraClass *pHF);
    BOOL VerifySamplePoints(HydraClass *pHF, DWORD sampleRAMSizeInBytes);
};

#endif
