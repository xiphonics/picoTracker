     /*********************************************************************
     
     sfnav.cpp
     
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

/******************************************************************************
*  @(#)sfnav.cpp	1.1 12:06:45 3/15/95 12:06:47
*
* Filename: sfnav.cpp
*
* Description:  The SoundFont Navigator and related methods
*
******************************************************************************
*/

#ifndef __SFNAV_CPP
#define __SFNAV_CPP

/**************
* Includes
**************/

#include "SFNAV.H"
#include "SFENUM.H"
#include "SFDATA.H"
#include "HYDRA.H"
#include "EMUERRS.H"
#include <math.h>
#include <string.h>

#include "SFLOOKUP.H"

using namespace::soundfont ;

/******************************************************************************
*
* Function: SoundFontNavigator::SoundFontNavigator
*
* Implemetation Notes:  Constructor for SoundFont object
*
********************************************************************************
*/
SoundFontNavigator::SoundFontNavigator(void)
{
  Reset();
  InitLookup();
}

/******************************************************************************
*
* Function: SoundFontNavigator::~SoundFontNavigator
*
* Implemetation Notes:  Destructor for SoundFont object
*
********************************************************************************
*/
SoundFontNavigator::~SoundFontNavigator(void)
{
  Reset();
  ClearLookup();
}

/******************************************************************************
*
* Function: SoundFontNavigator::Reset
*
* Implemetation Notes:  Initialization of the Navigator object
*
********************************************************************************
*/
void  SoundFontNavigator::Reset(void)
{
  phfNav          = NULL;
  wOsc           = 0;
}

/******************************************************************************
*
* Function: SoundFontNavigator::Navigate
*                               For SoundFont 2.0
*
* Implemetation Notes:  Get articulation data from a SoundFont in memory
*
*******************************************************************************
*/

void  SoundFontNavigator::Navigate(WORD uiSFID, WORD uiKey, WORD uiVel)
{

  WORD  pbagIndex, pgenIndex, ibagIndex, igenIndex;

  WORD  uiPBagNdxL, uiPBagNdxH, uiIBagNdxL, uiIBagNdxH;
  WORD  uiPGenNdxL, uiPGenNdxH, uiIGenNdxL, uiIGenNdxH;
  WORD  uiGenOper,  uiInstGenOper;
  SHORT   iGenAmt,    iInstGenAmt;
  BOOL   bGlobalPrstParams, bGlobalInstParms;
  sfData *psfVectorCurrOsc;

  SFSAMPLEHDRPTR pshSHdrCurrSmpl;
  SFGENLISTPTR   psfCurrPGen;
  SFGENLISTPTR   psfCurrIGen;

  const SFLOOKUP * soundFontLookup;

  if (phfNav == NULL)
    return;

  if ((soundFontLookup = GetLookup()) == NULL)
    return;

  if (uiSFID > (phfNav->awStructSize[prstHdr] - 1))
    return;

  //////////////////////////////////////////////////////////
  // We begin navigation at the Preset Header index 
  // set up by GetSFNum() and stored in uiSFID, in the SoundFont 
  // instance (or HydraClass pointer) setup by SetHydraFont() 
  // and stored in phfNav. See SetHydraFont(), GetSFNum()
  //////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////
  // The distance from pPHdr[uiSFID] to pPHdr[uiSFID + 1]   
  // indicates the number of LAYERS for the current PRESET
  //////////////////////////////////////////////////////////
  uiPBagNdxL = phfNav->pPHdr[uiSFID].wBagNdx;     // Set the indices for this
  uiPBagNdxH = phfNav->pPHdr[uiSFID + 1].wBagNdx; //   preset number
  wOsc      = 0;                          // Reset the # of Osc's needed

  // Assign the key and velocity values to the default vector 
  phfNav->sfDefault.shKeynum   = uiKey;
  phfNav->sfDefault.shVelocity = uiVel;

  //// Preset Layers ////

  //////////////////////////////////////////////////////////
  // Clear current layer data
  // Being ADDITIVE RELATIVE generators, the default value for all
  // PRESET generators MUST be the additive identity (0)
  //////////////////////////////////////////////////////////
  memset((void *)&sfPresetData, 0, sizeof(sfPresetData));
  memset((void *)linkFound, 0, MAX_SAMPLES);
  nextOscLinkCheck = 0;

  for (pbagIndex = uiPBagNdxL; pbagIndex < uiPBagNdxH; pbagIndex++)
  {
    //////////////////////////////////////////////////////////
    // The distance from pPBag[pbagIndex] to 
    // pPBag[pbagIndex + 1] indicates the      
    // number of GENERATORS for the current LAYER
    //////////////////////////////////////////////////////////
    uiPGenNdxL = phfNav->pPBag[pbagIndex].wGenNdx;
    uiPGenNdxH = phfNav->pPBag[pbagIndex + 1].wGenNdx;
    bGlobalPrstParams = TRUE;

    //// Set the Current SoundFont Layer to the Default ////
    sfCurrPreset = sfPresetData;

    //// Preset Generator operators ////

    for (pgenIndex = uiPGenNdxL; pgenIndex < uiPGenNdxH; pgenIndex++)
    {
      // Assign a pointer to the position of the current generator.
      // Reference the data from this pointer in the future code.
      // Makes for smaller and faster code.
      psfCurrPGen = &(phfNav->pPGen[pgenIndex]);
      uiGenOper = psfCurrPGen->sfGenOper;
      iGenAmt   = psfCurrPGen->unAmt.shAmount;

      /////////////////////////////////////////////////////
      // Overriding keynum/velocity found at LAYER level
      // Overriding keynum/velocity here affects LAYER and 
      // future INSTRUMENT split decisions ONLY! They are NOT
      // RELATIVE values to be added to the INSTRUMENT
      // or DEFAULT keynum/velocity
      /////////////////////////////////////////////////////
      if (uiGenOper == keynum) 
        ;//uiKey = iGenAmt;

      else if (uiGenOper == velocity) 
	;//uiVel = iGenAmt;

      /////////////////////////////////////////////////////
      // Check the key and velocity ranges
      /////////////////////////////////////////////////////

      else if ((uiGenOper == keyRange) && 
              ((uiKey < psfCurrPGen->unAmt.stRange.byLo) ||
               (uiKey > psfCurrPGen->unAmt.stRange.byHi)))
      {
        /////////////////////////////////////////////////////
        // A keyrange was detected, however the current key does not lie
        // within that range. Off to the next PBAG.
        /////////////////////////////////////////////////////
        bGlobalPrstParams = FALSE;
	break;
      }

      else if ((uiGenOper == velRange) && 
                  ((uiVel < psfCurrPGen->unAmt.stRange.byLo) ||
                   (uiVel > psfCurrPGen->unAmt.stRange.byHi)))

      {
        /////////////////////////////////////////////////////
        // A velocity range was detected, however the current velocity does 
        // not lie within that range. Off to the next PBAG.
        /////////////////////////////////////////////////////
        bGlobalPrstParams = FALSE;
	break;
      }

      else if (uiGenOper == instrument)
      {

        //////////////////////////////////////////////////////////
	// sGenAmt equals the Instrument token, that means
        // an instrument was found within the layer. This is the terminal
        // token which contains information for all samples and split level
        // articulation parameters 
        //////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////
        // The distance from pInst[iGenAmt] to 
        // pInst[iGenAmt + 1] indicates the      
        // number of GENERATOR SPLITS for the current INSTRUMENT
        //////////////////////////////////////////////////////////
	uiIBagNdxL = phfNav->pInst[iGenAmt].wBagNdx;
	uiIBagNdxH = phfNav->pInst[iGenAmt + 1].wBagNdx;

	//// Assign the current vector pointer ////
	psfVectorCurrOsc = &sfVector[wOsc];

	// Set the current vector to the DEFAULT vector
	*psfVectorCurrOsc = phfNav->sfDefault;

        // Set overriding key/vel given at the layer
        psfVectorCurrOsc->shKeynum   = uiKey;
        psfVectorCurrOsc->shVelocity = uiVel;

        // Copy current to default instrument
        // This data vector contains (or will contain) current default 
        // generator values plus the possibly upcoming Global Split generators.
	sfInstData = *psfVectorCurrOsc;

	for (ibagIndex = uiIBagNdxL; ibagIndex < uiIBagNdxH; ibagIndex++)
	{

          //////////////////////////////////////////////////////////
          // The distance from pInst[iGenAmt] to 
          // pInst[iGenAmt + 1] indicates the      
          // number of GENERATORS for the current SPLIT
          //////////////////////////////////////////////////////////
	  uiIGenNdxL = phfNav->pIBag[ibagIndex].wGenNdx;
	  uiIGenNdxH = phfNav->pIBag[ibagIndex + 1].wGenNdx;
	  bGlobalInstParms = TRUE;

	  //// Check for instrument Multisamples Generator Operators ////

	  for (igenIndex = uiIGenNdxL; igenIndex < uiIGenNdxH; igenIndex++)
	  {
            // Assign a pointer to the position of the current generator.
            // Reference the data from this pointer in the future code.
            // Makes for smaller and faster code.
	    psfCurrIGen = &(phfNav->pIGen[igenIndex]);

	    //// uiIBagNdx is an index to the phfNav->pIGen array ////
	    uiInstGenOper = psfCurrIGen->sfGenOper;
	    iInstGenAmt   = psfCurrIGen->unAmt.shAmount;
  
            // Check for overriding key/vel

	    if (uiInstGenOper == keynum)
	      uiKey = iGenAmt;

	    else if (uiInstGenOper == velocity)
	      uiVel = iGenAmt;

	    //// Check for appropriate key and velocity ranges ////

	    if ((uiInstGenOper == keyRange) || (uiInstGenOper == velRange))
	    {
	      ///////////////////////////////////////////////
	      // If we have several keyRanges/velRanges,   //
	      // and the range does not qualify, we do NOT //
	      // want the sfVector[wOsc] copied to the    //
	      // sfInstData vector, hence:                 //
	      // bGlobalInstParms = FALSE;                 //
	      ///////////////////////////////////////////////

	      if (uiInstGenOper == keyRange)
	      {
	        if ((uiKey < psfCurrIGen->unAmt.stRange.byLo) ||
		    (uiKey > psfCurrIGen->unAmt.stRange.byHi))
	        {
		  bGlobalInstParms = FALSE;
		  break;
	        }
	      }

	      else if ((uiVel < psfCurrIGen->unAmt.stRange.byLo) ||
			    (uiVel > psfCurrIGen->unAmt.stRange.byHi))
              {
	        bGlobalInstParms = FALSE;
	        break;
	      }

	      //////////////////////////////////////////////////////
	      // An instrument listed in Preset layer was found
	      // Since an instrument is DEFINED as the terminal 
	      // token for a single layer, this assignment avoids
              // redundant future loops
	      //////////////////////////////////////////////////////
	      pgenIndex = uiPGenNdxH;
	    }

	    // Check for sampleId
	    else if (uiInstGenOper == sampleId)
	    {

	      //////////////////////////////////////////////////////
              // A sample ID was found. This is the terminal token for a
              // single split which contains the index to the sample 
              // information.
	      //////////////////////////////////////////////////////

              // Assign a pointer to the position of the current sample header.
              // Reference the data from this pointer in the future code.
              // Makes for smaller and faster code.
	      pshSHdrCurrSmpl = &phfNav->pSHdr[iInstGenAmt];

	      //////////////////////////////////////////////////////
	      // Establish sample links
              // Build an array of links based on SoundFont 
              // sample header indices, which will (later) be
              // converted to an array based on OUTPUT VECTOR
              // indices. (ProcessSampleLinks)
	      //////////////////////////////////////////////////////
	      if ((pshSHdrCurrSmpl->sfSampleType&0x7FFF) > monoSample)
	      {
	        psfVectorCurrOsc->shSampleLink = pshSHdrCurrSmpl->wSampleLink;
	        shdrIndexLinks[wOsc] = iInstGenAmt;
		psfVectorCurrOsc->shSampleModes |= LINKED;
                if (pshSHdrCurrSmpl->sfSampleType == rightSample)
		  psfVectorCurrOsc->shSampleModes |= FIRST_LINK;
	      }
	      else
	      {
	        psfVectorCurrOsc->shSampleLink = 0;
		psfVectorCurrOsc->shSampleModes &= ~(FIRST_LINK | LINKED);
	      }

	      // Set the sample addresses accounting for coarse and fine
              // address offsets

	      psfVectorCurrOsc->dwStart =
	       (LONG) psfVectorCurrOsc->dwStart +
	       (LONG) psfVectorCurrOsc->shStartAddrsCoarseOffset * 32768 +
		      pshSHdrCurrSmpl->dwStart;

	      psfVectorCurrOsc->dwEnd =
	       (LONG) psfVectorCurrOsc->dwEnd +
	       (LONG) psfVectorCurrOsc->shEndAddrsCoarseOffset * 32768 +
		      pshSHdrCurrSmpl->dwEnd;
   
	      psfVectorCurrOsc->dwStartloop =
	       (LONG) psfVectorCurrOsc->dwStartloop +
	       (LONG) psfVectorCurrOsc->shStartloopAddrsCoarseOffset * 32768 +
		      pshSHdrCurrSmpl->dwStartloop;
   
	      psfVectorCurrOsc->dwEndloop =
	       (LONG) psfVectorCurrOsc->dwEndloop +
	       (LONG) psfVectorCurrOsc->shEndloopAddrsCoarseOffset * 32768 +
		      pshSHdrCurrSmpl->dwEndloop;

              // Obtain the sample rate, root key and sample tuning correction

	      psfVectorCurrOsc->dwSampleRate = pshSHdrCurrSmpl->dwSampleRate;

	      twoByteUnion originalKeyAndCorrection;

	      if ((psfVectorCurrOsc->shOverridingRootKey >=   0) &&
	          (psfVectorCurrOsc->shOverridingRootKey <  128))
	        originalKeyAndCorrection.byVals.by1 =
			   BYTE(psfVectorCurrOsc->shOverridingRootKey);

	      else if (pshSHdrCurrSmpl->byOriginalKey <  128)
	        originalKeyAndCorrection.byVals.by1 =
			   pshSHdrCurrSmpl->byOriginalKey;

              else
                originalKeyAndCorrection.byVals.by1 = 60; 

	      originalKeyAndCorrection.byVals.by0 = 
			   (BYTE)pshSHdrCurrSmpl->chFineCorrection;

	      psfVectorCurrOsc->shOrigKeyAndCorr =
                           (SHORT)originalKeyAndCorrection.wVal;

	      // Add in preset level changes
	      // Result is in psfVectorCurrOsc.
	      AddSoundFonts(psfVectorCurrOsc, &sfCurrPreset);

	      wOsc++;
	      psfVectorCurrOsc = &(sfVector[wOsc]);

	      // Do not exceed the allowable samples.
              // As a compromise between quick single-note throughput
              // and decent single-note polyphony, this code limits the
              // total number of SAMPLES PER NOTE-ON to MAX_SAMPLES.
	      if (wOsc == MAX_SAMPLES)
	      {
	        ProcessSampleLinks();
	        return;
	      }

	      // Let's not do this if we are certain of not getting another 
              // vector in this bag.
	      else if (ibagIndex != (uiIBagNdxH - 1))
	      {
                // Copy the contents of the DEFAULT INSTRUMENT generators
                // to the NEXT Vector.
	        *psfVectorCurrOsc = sfInstData;
	      }

	      // Reset previous overriding keynumbers
	      uiKey = sfInstData.shKeynum;
	      uiVel = sfInstData.shVelocity;

              ///////////////////////////////////////////////////
              // This navigator automatically handles sample loop
              // point offsets and root key definitions. So we now
	      // zero out those parameters in previous vector which 
              // have been already processed
              ///////////////////////////////////////////////////

              sfVector[wOsc-1].shStartAddrsCoarseOffset = 0;
              sfVector[wOsc-1].shEndAddrsCoarseOffset = 0;
              sfVector[wOsc-1].shStartloopAddrsCoarseOffset = 0;
              sfVector[wOsc-1].shEndloopAddrsCoarseOffset = 0;
              sfVector[wOsc-1].shOverridingRootKey = -1;

              bGlobalInstParms = FALSE;

	    }
	    else
	    {
              ///////////////////////////////////////////////////
              // A single step lookup/assignment to get the 
              // generator value into the output vector.
              // NOTE this will cause problems if the generator
              // value exceeds the maximum defined value within
              // this software! Hence the sanity checking at load
              // time for illegal generators.
              ///////////////////////////////////////////////////

	      // This step necessary for Big Endian systems
	      if (uiInstGenOper <= endloopAddrsOffset)
		*(LONG*)((BYTE*)&(psfVectorCurrOsc->dwStart) +
                   soundFontLookup[uiInstGenOper]) = (LONG)iInstGenAmt;
              else
	        *(SHORT*)((BYTE*)&(psfVectorCurrOsc->dwStart) +
                   soundFontLookup[uiInstGenOper]) = iInstGenAmt;
	    }
	  } // Loop through all Generators in a SPLIT

          /////////////////////////////////////////////////////////////
          // A future loop for SoundFont MODULATOR SPLITS
          // will be inserted here
          // IE from uiIModNdxL = phfNav->pIBag[ibagIndex].wModNdx;
          //    to   uiIModNdxH = phfNav->pIBag[ibagIndex + 1].wModNdx;
          /////////////////////////////////////////////////////////////

          ///////////////////////////////////////////////////
          // If the split we found was a GLOBAL split, then
          // its parameters must be INCLUDED in the DEFAULT
          // INSTRUMENT vector also.
          ///////////////////////////////////////////////////
	  if (bGlobalInstParms)
	  {
            sfInstData = *psfVectorCurrOsc;
	  }
        } // Loop through all SPLITS of Generators and Modulators

        ///////////////////////////////////////////////////////////////////
        // Here the sample links in the previous instrument are converted 
        // from Sample Header indices (0 to number of SAMPLES in the BANK) 
        // to output vector indices (0 to MAX_SAMPLES-1)
        // Note only samples within a single instrument may be linked!
        ///////////////////////////////////////////////////////////////////
        ProcessSampleLinks();

        // If we found an Instrument, this is NOT the global layer.
        bGlobalPrstParams = FALSE;

      } // Instrument found

      else
      {
        ///////////////////////////////////////////////////
        // A single step lookup/assignment to get the 
        // generator value into the output vector.
        // NOTE this will cause problems if the generator
        // value exceeds the maximum defined value within
        // this software! Hence the sanity checking at load
        // time for illegal generators.
        ///////////////////////////////////////////////////

        // Necessary for 'byte-incoherent' systems
        if (uiGenOper <= endloopAddrsOffset)
          *(LONG*)((BYTE*)&sfCurrPreset.dwStart +
            soundFontLookup[uiGenOper]) = (LONG)iGenAmt;
        else
          *(SHORT*)((BYTE*)&sfCurrPreset.dwStart +
            soundFontLookup[uiGenOper]) = iGenAmt;
      }

    } // Loop through all Generators in a LAYER

    //// Preset Modulator operators ////

    ///////////////////////////////////////////////////////////////
    // A future loop for SoundFont MODULATOR LAYERS
    // will be inserted here
    // IE from uiPModNdxL = phfNav->pPBag[pbagIndex].wModNdx;
    //    to   uiPModNdxH = phfNav->pPBag[pbagIndex + 1].wModNdx;
    ///////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    // If the layer we found was the GLOBAL layer, then
    // its parameters must be INCLUDED in the DEFAULT
    // LAYER vector also.
    // Overriding key/vel NOT found at the GLOBAL
    // LAYER should be restored to the input values.
    // Overriding key/vel FOUND at the GLOBAL LAYER
    // should REPLACE the input values
    ///////////////////////////////////////////////////
    if (bGlobalPrstParams)
    {
      sfPresetData = sfCurrPreset;
    }

  } // Loop through all LAYERS of Generators and Modulators 

  // The navigation process is finished.
  return;
}

/******************************************************************************
*
* Function: SoundFontNavigator::ProcessSampleLinks
*
* Implemetation Notes:  Convert sample ID based links to vector index based
*                       links
*******************************************************************************
*/

void SoundFontNavigator::
ProcessSampleLinks(void)
{
  sfData *pCheckSF, *pCurrSF;
  BOOL found;
  WORD vectorIndexLinks[MAX_SAMPLES], saveStart = nextOscLinkCheck;

  // 
  // The idea here is that a LINKED sampled group is defined as
  // two or more samples which WERE FOUND within the SPLIT or LAYER lists in
  // a preset and whose sfSampleTypes field was set to leftSample, or 
  // rightSample, or linkSample, AND whose sfSampleLink fields circularly
  // point to each other. 
  //
  // A STEREO Pair is a Linked sample group of TWO samples.
  // Surround-sound could be implemented as a linked sample group of 
  // FIVE (or six) samples (if supporting hardware exists...)
  //
  // The ONLY audio feature that the STEREO/LINKED samples specifies is 
  // that their pitch amounts must be exactly the same at all times. 
  // (Equal sample rates, NO phase difference between the samples!)
  //
  // This means one oscillator controls the pitch amounts and pitch
  // modulation for all of the linked oscillators. Stereo/Linked 
  // samples do NOT need to have specific pan values (although typically 
  // a stereo pair would be panned right/left) or filter values or whatever. 
  // So you CAN have a "stereo" pair where one sample is being filtered
  // differently or has more trememlo (LFO->volume) than the other.
  //
  // In a stereo or linked sample list, the RIGHT sample controls the pitch,
  // the LEFT or LINKED samples follows the pitch of the RIGHT sample.
  // So in effect, any generators or modulators found in a split with
  // a LEFT or LINKED sample which modulate pitch in any way  (IE:
  // LFO->pitch, Env->pitch, etc) are NOT used if the stereo pair or the 
  // entire link is found in naviagtion.
  //
  // This is a linked sample group.
  // Two vectors (with one sample each) were found upon naviation. (wOsc == 2) 
  // Therefore it is a stereo pair.
  // Sample1 is Right sample, link points to Sample2
  // Sample2 is Left sample,  link points to Sample1
  //
  // This is NOT a linked sample group (nor is it a stereo pair)
  // Two vectors (with one sample each) were found upon naviation. (wOsc == 2)
  // Sample1 is Right sample, link points to Sample2
  // Sample3 is Mono sample,  link point is undefined
  // The samples do not point to each other
  //
  // This is NOT a linked sample group (nor is it a stereo pair)
  // Two vectors (with one sample each) were found upon naviation. (wOsc == 2)
  // There are two vectors (wOsc == 2), one with Sample2, other with Sample4
  // Sample2 is Left sample,  link points to Sample1
  // Sample4 is Right sample,  link points to Sample5
  // The samples are Left and Right samples but NOT to each other
  //
  // IE: This is a LINKED sample group
  // Four vectors (with one sample each) were found upon naviation. (wOsc == 4)
  // Sample10 is Right sample, link points to Sample11
  // Sample11 is Linked sample, link points to Sample12
  // Sample12 is Linked sample, link points to Sample13
  // Sample13 is Linked sample,  link points to Sample10
  //
  // IE: This is NOT a LINKED sample group
  // Three vectors (with one sample each) were found upon naviation. (wOsc == 3)
  // Sample10 is Right sample, link points to Sample11
  // Sample11 is Linked sample, link points to Sample12
  // Sample12 is Linked sample, link points to Sample13
  // The circle is not complete.
  //

  // What is happening here is that all vectors are being scanned to see
  // if stereo and/or linked samples exist AND point to each other

  // The array 'shdrIndexLinks' was filled by the Navigate() routine. It
  // contains a list of which SAMPLE INDEX each
  // vector's sample points to. A second array called 'vectorIndexLinks' is 
  // stuffed with the corresponding VECTOR indices, thus leaving all data
  // in tact until all processing is completed.

  for (WORD count = nextOscLinkCheck; count < wOsc; count++, nextOscLinkCheck++)
  {
    pCurrSF = &sfVector[count];
    found = FALSE;
    // Is this a linked sample?
    if (pCurrSF->shSampleModes & LINKED)
    {
      // If so, find its link
      for (WORD count1 = saveStart; count1 < wOsc; count1++)
      {
        pCheckSF = &sfVector[count1];
  
        if ((pCheckSF->shSampleModes & LINKED)              &&
            (shdrIndexLinks[count1] == pCurrSF->shSampleLink) &&
            (linkFound[count1] == 0))
        {
         // Temporarily store the vector link points 
         vectorIndexLinks[count] = count1;

         // Flag the fact that this vector has been found as a link!
         linkFound[count1] = 1;

         found=TRUE;
         break;
        }
      }

      // If there was no link, then turn off the LINK flags on the CURRENT
      // sample under consideration. Note that if a chain of linked samples
      // is found, this may not follow and reset the entire chain
      // ALL of the way back to the beginning of a list. Thus misleading
      // vectors may be produced. 
      //
      // Say x points to y and y points to z and z points to x. But in this
      // navigation only x and y were found. This process will return saying
      // x is a valid link to y, but y is an invalid link.
      //
      // There is no issue with an two unlinked samples which are linked 
      // elsewhere. 
      //
      // Say a points to b and c points to d. In this navigation a and d 
      // were found. Both a and d will come out saying they have no valid
      // links.

      if (found == FALSE)
      {
	pCurrSF->shSampleModes &= ~(FIRST_LINK|LINKED);
	pCurrSF->shSampleLink = 0;
      }
    }
  }

  // Now assign the each VECTOR's link value to the values which the
  // above stored in the 'vectorIndexLinks' array.
  for (int count = saveStart; count < wOsc; count++)
  {
    pCurrSF = &sfVector[count];
    if ((pCurrSF->shSampleModes & LINKED) == LINKED)
        pCurrSF->shSampleLink = vectorIndexLinks[count];
  }

  // Now, all vectors have shSampleLink set to the values of the 
  // corresponding VECTOR indices instead of the sample indices.
  // Each vector's shSampleType field has the 3rd bit set indicating 
  // that it IS a link, and one vector's shSampleType field ALSO has the
  // 4th bit set indicating that is the FIRST link.

  // If the search above found that vectors with linked samples did NOT
  // have their counterparts in the final array of vectors, then all 
  // the shSampleType field get bits 3 and 4 reset. 
}

/******************************************************************************
*
* Function: SoundFontNavigator::GetSFNum
*
* Implemetation Notes:  Return value associated with a single bank/preset
*                       pair in the CURRENT SoundFont to outside world
*
*                       This sets up the index which will be used in the
*                       Navigate() routine. With this being a seperate call,
*                       all Navigation which is ONLY required upon a 
*                       Bank/Patch select call is done once only.
*
********************************************************************************
*/
WORD  SoundFontNavigator::GetSFNum(WORD wBank, BYTE byPatch, WORD* pwSFID)
{
  WORD wSize = phfNav->awStructSize[prstHdr]-1;

  if (phfNav == NULL)
    return(SF_INVALIDBANK);

  for (WORD wNdx = 0; wNdx < wSize; wNdx++)
  {
    if ((phfNav->pPHdr[wNdx].wPresetBank == wBank) &&
	(phfNav->pPHdr[wNdx].wPresetNum  == byPatch))
    {
      *pwSFID = wNdx;
      return (SUCCESS);
    }
  }
  return (SF_PRESETNOTFOUND);
}

/******************************************************************************
*
* Function: SoundFontNavigator::GetHydraFont
*
* Implemetation Notes:  Return a pointer to the CURRENTLY SET SoundFont instance
*                       (see SetHydraFont)
*
********************************************************************************
*/

void  SoundFontNavigator::GetHydraFont(HydraClass* pHydra)
{
  pHydra = phfNav;
  return;
}


/*******************************************************************
* SetHydraFont: Set the in memory instance of a SoundFont (pet-named
*               a HydraFont or a HydraClass) as the one to be used
*               in all future SoundFontNavigator calls. 
*
*               With this being a seperate call,
*               all setup which is ONLY required upon a 
*               Bank select or some other change in the SoundFont 
*               instance (such as unload bank, then load other bank)
*               can be done once only.
*
* The function returns a pointer to the PREVIOUSLY set SoundFont
* in case you need it...
*******************************************************************
*/
HydraClass*  SoundFontNavigator::SetHydraFont(HydraClass* pHydra)
{

  HydraClass * pOldHydra;
  pOldHydra = phfNav;
  phfNav = pHydra;

  return(pOldHydra);
}

/*******************************************************************
* AddSoundFonts: The layer/split addition operation function. Simply
*                take all elements in the final layer vector and add 
*                them to the final split vector.
*
*                The summed vectors is returned in sfSoundFontReturned
*********************************************************************
*/
void
SoundFontNavigator::AddSoundFonts(sfData * sfSoundFontReturned, 
                                  sfData * sfSFPresetAdd)
{
  WORD wCount, uiOffset;
  const SFLOOKUP *soundFontLookup;

  if ((soundFontLookup = GetLookup()) == NULL)
    return;

  for (wCount = 0; wCount < endOper; wCount++)
  {
    uiOffset = (WORD) soundFontLookup[wCount];

    *(SHORT*)((BYTE*)&(sfSoundFontReturned->dwStart) + uiOffset) +=
	*(SHORT*)((BYTE*)&(sfSFPresetAdd->dwStart) + uiOffset);
  }
}

#endif //  __SFNAV_CPP
