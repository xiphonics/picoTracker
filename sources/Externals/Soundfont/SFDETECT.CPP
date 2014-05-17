     /*********************************************************************
     
     sfdetect.cpp
     
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
* @(#)sfdetect.cpp	1.2 13:56:04 3/22/95 13:56:06
*                          
*  FILE :   sfdetect.cpp
*
*  Description: 
* 
*  Search the Loaded HydraClass object for internal errors, repair if
*  desired and possible
*
*============================================================================
*/
#include "SFDETECT.H"
#include "SFENUM.H"

typedef struct sampleSpaceNodeTag
{
  DWORD start;
  DWORD end;
  struct sampleSpaceNodeTag *next;
} sampleSpaceNode;

/*============================================================================
*  VerifySFBData: Given a HydraClass object, search for internal errors.
*============================================================================
*/

BOOL sfDetectErrors::
VerifySFBData(HydraClass *pHF, DWORD sampleRAMSize)
{
  if(VerifyPDTAIndices(pHF) == FALSE) return FALSE;
  if(VerifySamplePoints(pHF, sampleRAMSize) == FALSE) return FALSE;
  return TRUE;
}

/*============================================================================
*  VerifyPDTAIndices: Given a HydraClass object, search for bad index values
*                     in the preset data.
*============================================================================
*/
BOOL sfDetectErrors::
VerifyPDTAIndices(HydraClass *pHF)
{

  WORD count, lastPBag, lastPGen, lastIBag, lastIGen;
  WORD lastPMod, lastIMod;

  // Step 0: Make sure all struct array counts are at least the minimum size.
  if (pHF->awStructSize[prstHdr]     < 2) return FALSE;
  if (pHF->awStructSize[prstBagNdx]  < 1) return FALSE;
  if (pHF->awStructSize[prstGenList] < 1) return FALSE;
  if (pHF->awStructSize[prstModList] < 1) return FALSE;
  if (pHF->awStructSize[inst]        < 2) return FALSE;
  if (pHF->awStructSize[instBagNdx]  < 1) return FALSE;
  if (pHF->awStructSize[instGenList] < 1) return FALSE;
  if (pHF->awStructSize[instModList] < 1) return FALSE;
  if (pHF->awStructSize[sampHdr]     < 1) return FALSE;

  // Step 1: Look for generator or modulator enums which are not in the
  //         valid range. This is a fixable corruption, in fact it may
  //         represent a FUTURE SoundFont. 
  //         If desired, change those which are exceed the maximum to NOP.
  //         Also, check out the INSTRUMENT and SAMPLE indices and verify
  //         their validity

  // Loop to struct size-2 because: 1. last index = struct size - 1, 
  //                                2. last index is the terminal and
  //                                   need not be considered

  // First the PGens...
  for (count = 0; count < pHF->awStructSize[prstGenList]-2; count++)
  {
    // This code is already in the sfReader. For an editor, this function
    // may be better served here.
    //if (pHF->pPGen[count].sfGenOper > endOper)
    //{
    //  pHF->pPGen[count].sfGenOper = nop;
    //}

    if (pHF->pPGen[count].sfGenOper == instrument)
    {
      if (pHF->pPGen[count].unAmt.shAmount > pHF->awStructSize[inst]-1)
      {
        // We have found an bad instrument index. 
        return FALSE;
      }
    }
  }

  // Now the IGens...
  for (count = 0; count < pHF->awStructSize[instGenList]-2; count++)
  {
    //if (pHF->pIGen[count].sfGenOper > endOper)
    //{
    //    pHF->pIGen[count].sfGenOper = nop;
    //}

    if (pHF->pIGen[count].sfGenOper == sampleId)
    {
      if (pHF->pIGen[count].unAmt.shAmount > 
                                     pHF->awStructSize[sampHdr]-1)
      {
        // We have found an bad sample index. 
        return FALSE;
      }
    }
  }

  //*************************************************************
  // Future loops for PMOD and IMOD will go here when necessary
  //*************************************************************

  // Step 2: Look for Preset Layer indices which exceed the maximum

  lastPBag = 0;
  for (count = 0; count < pHF->awStructSize[prstHdr]-2; count ++)
  {
    if ((pHF->pPHdr[count].wBagNdx > pHF->awStructSize[prstBagNdx] - 1) ||
        (pHF->pPHdr[count].wBagNdx < lastPBag))
    {
      // We have found a bad preset layer index. 
      return FALSE;
    }
    lastPBag = pHF->pPHdr[count].wBagNdx;
  }

  // Step 3: Look for bad Generator List and Modulator List Indices

  lastPGen = lastPMod = 0;
  for (count = 0; count < pHF->awStructSize[prstBagNdx]-2; count ++)
  {
    if ((pHF->pPBag[count].wGenNdx > pHF->awStructSize[prstGenList] - 1) ||
        (pHF->pPBag[count].wGenNdx < lastPGen))
    {
      // We have found a bad Generator List index.
      return FALSE;
    }
    lastPGen = pHF->pPBag[count].wGenNdx;

    if ((pHF->pPBag[count].wModNdx > pHF->awStructSize[prstModList] - 1) ||
        (pHF->pPBag[count].wModNdx < lastPMod))
    {
      // We have found a bad Modulator index.
      return FALSE;
    }
    lastPMod = pHF->pPBag[count].wModNdx;
  }

  // Step 3: Look for Instrment split indices which exceed the maximum

  lastIBag = 0;
  for (count = 0; count < pHF->awStructSize[inst]-2; count ++)
  {
    if ((pHF->pInst[count].wBagNdx > pHF->awStructSize[instBagNdx] - 1) ||
	(pHF->pInst[count].wBagNdx < lastIBag))
    {
      // We have found a bad instrument split index
      return FALSE;
    }
    lastIBag = pHF->pInst[count].wBagNdx;
  }

  // Step 4: Look for Instrument Generator List and Modulator List Indices
  //         which exceed the maximum.

  lastIGen = lastIMod = 0;
  for (count = 0; count < pHF->awStructSize[instBagNdx]-2; count ++)
  {
    if ((pHF->pIBag[count].wGenNdx > pHF->awStructSize[instGenList] - 1) ||
	(pHF->pIBag[count].wGenNdx < lastIGen))
    {
      // We have found a bad generator index 
      return FALSE;
    }
    lastIGen = pHF->pIBag[count].wGenNdx;

    if ((pHF->pIBag[count].wModNdx > pHF->awStructSize[instModList] - 1) ||
	(pHF->pIBag[count].wModNdx < lastIMod))
    {
      // We have found a bad modulator index 
      return FALSE;
    }
    lastIMod = pHF->pIBag[count].wModNdx;
  }

  // We have no index problems.
  return TRUE;
}

/*============================================================================
*  VerifySamplePoints: Given a HydraClass object, search for bad index values
*                      sample points
*============================================================================
*/
BOOL sfDetectErrors::
VerifySamplePoints(HydraClass *pHF, DWORD sampleSizeInBytes)
{
  WORD count, count1;
  DWORD start, end, startloop, endloop;
  BOOL validity, validity1;

  // Here we are looking for invalid loop points. A valid samples worth
  // of loop points looks like the following (not to scale):
  //
  //        =====================================.......
  //        |       |                 |         |  46  |
  //        =====================================.......
  //       start  startloop         endloop    end   next start
  //
  // Start is the beginning of the sample data, end is the end of the sample
  // data. Start must be less startloop, which must be less than endloop
  // which must be less than end. The distance between startloop and endloop
  // should be at least 32 samples, but is not enforced by this function.
  // Finally, the start point of the following
  // sample must be at least 46 samples away from the end point of the 
  // current sample.
  //
  // Sample header data need NOT be in sequencial order, so the last 
  // stipulation is checked later.

  for (count = 0; count < pHF->awStructSize[sampHdr]-1; count++)
  {
    validity = FALSE;

    start     = pHF->pSHdr[count].dwStart;
    end       = pHF->pSHdr[count].dwEnd;
    startloop = pHF->pSHdr[count].dwStartloop;
    endloop   = pHF->pSHdr[count].dwEndloop;

    // SamplePointOffsets may allow some of these conditions, but
    // the sample information itself does not.
    if (startloop < start)          break;
    if (endloop   < startloop)      break;
    // if ((endloop - startloop) < 32) break;
    if (end       < endloop)        break;

    // If this is a ROM sample, we cannot detect for sample size limitations
    // without specific synthesizer information.
    if (((pHF->pSHdr[count].sfSampleType & romSample) == 0) &&
	 (end > sampleSizeInBytes/2))
      break;

    validity = TRUE;
  }

  if (validity == FALSE) 
    return validity;

  // In this loop, we check for overlapping samples. IE samples which 
  // by themselves are OK but lie inside the space of another sample.
  //
  //        =====================================.......
  //        |       |      32         |         |  46  |
  //        =====================================.......
  //       start  startloop         endloop    end   next start
  //
  //                       =====================================.......
  //                       |       |      32         |         |  46  |
  //                       =====================================.......
  //                       start  startloop         endloop    end   next start
  //
  // We must be able to take it for granted that all samples by themselves
  // are valid before making this check.
  // Note that overlap must not occur even in the buffer between end and 
  // next start. Finally, since sequential sample header data is not 
  // mandatory in a SoundFont bank, we need to use the two loop approach.
 

  for (count = 0; count < pHF->awStructSize[sampHdr]-1; count++)
  {
    validity = FALSE;

    for (count1 = 0; count1 < pHF->awStructSize[sampHdr]-1; count1++)
    {
      validity1 = FALSE;
      if ((count != count1)
	  &&
	  (pHF->pSHdr[count].dwStart <  pHF->pSHdr[count1].dwEnd + SAMPLEBUFFER)
          &&
          (pHF->pSHdr[count].dwStart >= pHF->pSHdr[count1].dwStart))
        break;
      validity1 = TRUE;
    }

    if (validity1 == FALSE) break;
    validity = TRUE;
  }

  return validity;
}
 
    
