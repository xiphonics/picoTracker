     /*********************************************************************
     
     sfreader.cpp
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
/*============================================================================
*
*                          Copyright (c) 1994
*               E-mu Systems Proprietary All rights Reserved.
*                             
*============================================================================
*/

#include "SFREADER.H"
#include "WIN_MEM.H"

//#define DEBUG

#ifdef DEBUG
#undef DEBUG
#endif


#include <stdlib.h>

using namespace::soundfont ;

/*============================================================================
* @(#)sfreader.cpp	1.2 13:56:04 3/22/95 13:56:06
*                          
*  FILE :   sfreader.c
*
*  Description: 
* 
* Load SoundFont articulation data, (w/o sample download) and return the address
* of an allocated HydraClass object. It also maintains
* an array of character strings read from the info chunk of the RIFF file.
*
*============================================================================
*/

#define SFR_NO_DATA    0xFF
#define SFR_ONDISK     0
#define SFR_ONMACDISK  3

#define SFR_CLOSED     0
#define SFR_OPEN       1

#define MAX_READ_SIZE 0x7FFE

////////////////////////////
//  The first Constructor
////////////////////////////
sfReader::sfReader(void) 
{
  ConstructSFReader();
} // end constructor 1


////////////////////////////
//  The Destructor
////////////////////////////
sfReader::~sfReader()
{
  DestructSFReader();
} /* end destructor */ 


//////////////////////////////////
// The first callable Constructor
//////////////////////////////////
void sfReader::ConstructSFReader(void)
{
  SetError(SUCCESS); 
  /* Private members */
  bySFDataLoc = SFR_NO_DATA;       // Flag to indicate storage medium
  bySFStorageStatus = SFR_CLOSED; // Flag to indicate if storage medium is open
  pathName = NULL;

  for(LONG i=0;i<CkCount;i++)  // Insures we are not 'delete'-ing any data
    infoCkValues[i] = NULL;   // which was not 'new'-ed.

  resetSampleCollector();
}


//////////////////////////////////////
// The callable Destructor
//////////////////////////////////////
void sfReader::DestructSFReader(void)
{
  resetInfoCkValues();

  if( pathName != NULL) { 
     delete [] pathName; 
     pathName = NULL; 
  }
}

/*
*****************************************************************************
*
* OpenSFB: Open up a SoundFont without loading or reading any data
*
*****************************************************************************
*/
void sfReader::
OpenSFBFile(CHAR * pFilename)
{
  SetError(SUCCESS);

  if (bySFStorageStatus == SFR_OPEN)
  {
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
  }

  resetInfoCkValues(); // dealloc InfoCkValues from last read file, (if any).

  if(pathName != NULL) {
      delete [] pathName;
      pathName = NULL;
  }

  if ((pathName = new CHAR[strlen(pFilename)+1]) == NULL)
  {
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return;
  }

  memset(pathName,0,strlen(pFilename) +1);
  strcpy(pathName,pFilename);

  if (tRIFF.OpenRIFF(pFilename) != SUCCESS) {
    SetError(EHC_OPENFILEERROR);
    return;
  }

  storeInfoCks();
  bySFDataLoc = SFR_ONDISK;
  bySFStorageStatus = SFR_OPEN;
}

void  sfReader::
CloseSFBFile(void)
{
  tRIFF.CloseRIFF();
  bySFStorageStatus = SFR_CLOSED;
}

HydraClass *sfReader::ReadSFBFile(CHAR* pFilename, CHAR * pchReqdWaveTable)
/*
*****************************************************************************
*
* ReadSFBFile: Setup for reading SoundFont from a file.
*****************************************************************************
*/
{

  SetError(SUCCESS);

  resetInfoCkValues(); // dealloc InfoCkValues from last read file, (if any).
	
  if(pathName != NULL) { 
      delete [] pathName;
      pathName = NULL;
  }

  if ((pathName = new CHAR[strlen(pFilename)+1]) == NULL)
  {
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  memset(pathName,0,strlen(pFilename) +1);
  strcpy(pathName,pFilename);
	
  if (tRIFF.OpenRIFF(pFilename) != SUCCESS) {
    SetError(EHC_OPENFILEERROR);
    return (NULL);
  }

  bySFDataLoc       = SFR_ONDISK;
  bySFStorageStatus = SFR_OPEN;

  return (ReadSFBData(pchReqdWaveTable));

}

#ifdef USE_MACINTOSH
HydraClass *sfReader::ReadSFBFile(FSSpec * pFSSpec, CHAR *pchReqdWaveTable)
/*
*****************************************************************************
*
* ReadSFBFile: Setup for reading SoundFont from a file.
*
*****************************************************************************
*/
{

  SetError(SUCCESS);

  resetInfoCkValues(); // dealloc InfoCkValues from last read file, (if any).

  pSpecifier = pFSSpec;

  if (tRIFF.OpenRIFF(pFSSpec) != SUCCESS) {
    SetError(EHC_OPENFILEERROR);
    return (NULL);
  }

  bBypassSFProcess = FALSE;
  bySFDataLoc = SFR_ONMACDISK;
  bySFStorageStatus = SFR_OPEN;

  return (ReadSFBData(pchReqdWaveTable));

}
#endif // USE_MACINTOSH

HydraClass* sfReader::
ReadSFBData (CHAR * pchReqdWaveTable)
/*****************************************************************************
* 
* Implementation Notes: 
*
*    This file reads the given SoundfontBank file into memory as a HydraClass
* object. Uses the class RIFFClass object used to read up the file and
* move through the various Chunks and sub chunks. It stores all the info
* chunk stuff in a global infoCkValues array, and the preset data into 
* a hydraClass object, it sticks the names into the last SampleHdr array 
* of the hydrafont.
*
*****************************************************************************
*/

{
 HydraClass *hf; 

  sfVersion *fversion = NULL;
  CHAR      *pchIROMChunk;
  WORD         uiCount;
  BOOL      bROMSamples = FALSE;

  if ((hf = new HydraClass) == NULL)
  {
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }
  tRIFF.Reset();
  resetSampleCollector();

  /*
   * Verify SoundFont data format
   */

  if ( (tRIFF.GetCkID() != MAKE_ID(tRIFF.GetRIFFToken(Riff) ) &&
       (tRIFF.GetCkFormID() != MAKE_ID(tRIFF.GetRIFFToken(Sfbk)) )))  
  {
	
       if(tRIFF.GetCkFormID() == MAKE_ID("SFBK")) /* all caps is a no no */
       {                                          /* accept it anyway   */ 

      /* cout<<"%ReadSFBK-I-found upcase SFBK fixing..."<<endl; */

       }                                 
       else {  /* really was a format error */ 

         delete hf;
         tRIFF.CloseRIFF();
         bySFStorageStatus = SFR_CLOSED;
	 SetError(EHC_RIFFERROR);
	 return (NULL);
       }
  }

  storeInfoCks(); /* stores all present infoSub chunks in the array */ 
		  /* infoCkValues...                                */
  
  fversion = (sfVersion*)(infoCkValues[Ifil]);

  if(fversion == NULL) { 
   // can't set the version of our hydraClass object. ifil is a _manditory_ 
   // info sub-chunk. We looked for "ifil" and "IFIL" (the second is illegal,
   // but does exist in otherwise valid banks, so we accept it as well....
   SetError(SF_INVALIDBANK);
   #ifdef DEBUG 
     cout<<"%ReadSFBFile()-E-Bad Format, ifil (file version)"
         <<" info subchunk missing"<<endl;
   #endif  
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
   return NULL;
  }

  hf->SetVersion(fversion->wMajor, fversion->wMinor);

  // Only reads SoundFont 2.x banks
  if (fversion->wMajor != 2)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(SF_INVALIDBANK);
    return NULL;
  }

  // Inam is a REQUIRED Info sub-chunk!
  if (hf->SetBankName(infoCkValues[Inam]) != SUCCESS) { 
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);                 
    return NULL;
  }

  /*
   *  Read SoundFont data into the 'nine heads'
   */

  /* Move to the Articulation Data chunk... */ 

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Pdta))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding PDTA chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }

  /* Allocate system memory for the Preset Table  */ 

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Phdr)))
  {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding PHDR chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }
  presetCnt = (WORD)((tRIFF.GetCkSize() / PRSTHDR_SIZE) - 1);

  hf->pPHdr = (SFPRESETHDRPTR) Allocate(prstHdr, sizeof(sfPresetHdr),
				      PRSTHDR_SIZE, tRIFF.GetCkSize() , hf);

  if (hf->pPHdr == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }
  /*  Move to pbag */

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Pbag))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding PBAG chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR );
    return (NULL);
  }
  hf->pPBag = (SFBAGNDXPTR)Allocate(prstBagNdx, sizeof(sfBagNdx),
				  PRSTBAGNDX_SIZE, tRIFF.GetCkSize(), hf);

  if (hf->pPBag == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  /* Move to pmod  */
  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Pmod))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding PMOD chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }

  hf->pPMod = (SFMODLISTPTR)Allocate(prstModList, sizeof(sfModList),
				PRSTMODLIST_SIZE, tRIFF.GetCkSize(), hf);
  if (hf->pPMod == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  /*  Move to pgen  */
  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Pgen))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding PGEN chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }

  hf->pPGen = (SFGENLISTPTR)Allocate(prstGenList, sizeof(sfGenList),
				 PRSTGENLIST_SIZE, tRIFF.GetCkSize(), hf);
  if (hf->pPGen == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  /* Move to inst */

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Inst)))  {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding INST chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }

  hf->pInst = (SFINSTPTR)Allocate(inst, sizeof(sfInst),
				INST_SIZE, tRIFF.GetCkSize(), hf );

  if (hf->pInst == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  /* Move to ibag  */

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Ibag))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding IBAG chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }

  hf->pIBag = (SFBAGNDXPTR)Allocate(instBagNdx, sizeof(sfBagNdx),
				INSTBAGNDX_SIZE, tRIFF.GetCkSize(), hf);

  if (hf->pIBag == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }
  /*  Move to imod  */

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Imod))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding IMOD chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }

  hf->pIMod = (SFMODLISTPTR) Allocate(instModList, sizeof(sfModList),
				   INSTMODLIST_SIZE, tRIFF.GetCkSize(), hf);

  if (hf->pIMod == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  /*  Move to igen  */
  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Igen))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding IGEN chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }
  hf->pIGen = (SFGENLISTPTR) Allocate(instGenList, sizeof(sfGenList),
				 INSTGENLIST_SIZE, tRIFF.GetCkSize(), hf);

  if (hf->pIGen == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }
  /*  Move to shdr */

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Shdr))) {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding SHDR chunk in " << pathName <<endl;
    #endif

    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_RIFFERROR);
    return (NULL);
  }
 
  hf->pSHdr = (SFSAMPLEHDRPTR) Allocate(sampHdr,               // kind
	                                  sizeof(sfSampleHdr),  // in mem size
 	   			          SAMPHDRV2_SIZE,      // file size
               				  tRIFF.GetCkSize(),  // chunk size 
                                          hf );              // update size

  if (hf->pSHdr == NULL)
  {
    delete hf;
    tRIFF.CloseRIFF();
    bySFStorageStatus = SFR_CLOSED;
    SetError(EHC_ALLOCATE_PMEM_ERROR);
    return(NULL);
  }

  /*
   *  Store sample size and/or offset for future use
   */

  if (SUCCESS != tRIFF.FindCk(tRIFF.GetRIFFToken(Smpl)))
  {
    #ifdef DEBUG
     cout<<"%ReadSFBFile-E-Error finding SMPL chunk in " << pathName <<endl;
    #endif
    dwSampleSize = 0;
    dwSampleOffset = 0;
  }
  else
  {
    dwSampleSize = tRIFF.GetCkSize();
    dwSampleOffset = 0;
  }

  /*
   * The following stuffs data structure members which were not in the
   * original SoundFont data structures. 
   */

  for (WORD x=0;x<hf->awStructSize[inst];x++){
     hf->pInst[x].wRefCount = 0;
  }
  for (WORD y=0;y<hf->awStructSize[sampHdr];y++){
     hf->pSHdr[y].wRefCount   = 0;
  }

  tRIFF.CloseRIFF();
  bySFStorageStatus = SFR_CLOSED;

  /*
   * Do Processing of SoundFont data based on system and audio hardware
   */

  /* Swap the order of the bytes in the data structs */

  for (uiCount = 0; uiCount < hf->awStructSize[prstHdr]; uiCount++) {

    tRIFF.SwapBytes(&hf->pPHdr[uiCount].wPresetNum);
    tRIFF.SwapBytes(&hf->pPHdr[uiCount].wPresetBank);
    tRIFF.SwapBytes(&hf->pPHdr[uiCount].wBagNdx);

    /* We now need to swap the DWORDS  */

  }
  for (uiCount = 0; uiCount < hf->awStructSize[prstBagNdx]; uiCount++) {

    tRIFF.SwapBytes(&hf->pPBag[uiCount].wGenNdx);
    tRIFF.SwapBytes(&hf->pPBag[uiCount].wModNdx);
  }
  for (uiCount = 0; uiCount < hf->awStructSize[prstGenList]; uiCount++){
    tRIFF.SwapBytes(&hf->pPGen[uiCount].sfGenOper);
    tRIFF.SwapBytes((WORD*)(&hf->pPGen[uiCount].unAmt));

    // The SoundFont 2.0 specification dictates that unknown generators
    // be ignored. We are doing that by changing its operator value to the
    // 'nop' operator for the purposes of the enabler. These lines may not
    // be desirable for an edit engine, which would want to leave data in
    // tact, and would instead want to be in a seperate verification routine.
    if (hf->pPGen[uiCount].sfGenOper > endOper)
      hf->pPGen[uiCount].sfGenOper = nop;
  }
  for (uiCount = 0; uiCount < hf->awStructSize[prstModList]; uiCount++) {
    tRIFF.SwapBytes(&hf->pPMod[uiCount].wModSrcOper);
    tRIFF.SwapBytes(&hf->pPMod[uiCount].wModDestOper);
    tRIFF.SwapBytes((WORD*)(&hf->pPMod[uiCount].shAmount));
  }
  /* The instrument layers */ 
  for (uiCount = 0; uiCount < hf->awStructSize[inst]; uiCount++) {
    tRIFF.SwapBytes(&hf->pInst[uiCount].wBagNdx);
  }
  for (uiCount = 0; uiCount < hf->awStructSize[instBagNdx]; uiCount++) {
    tRIFF.SwapBytes(&hf->pIBag[uiCount].wGenNdx);
    tRIFF.SwapBytes(&hf->pIBag[uiCount].wModNdx);
  }
  for (uiCount = 0; uiCount < hf->awStructSize[instGenList]; uiCount++) {
    tRIFF.SwapBytes(&hf->pIGen[uiCount].sfGenOper);
    tRIFF.SwapBytes((WORD*)(&hf->pIGen[uiCount].unAmt));

    // The SoundFont 2.0 specification dictates that unknown generators
    // be ignored. We are doing that by changing its operator value to the
    // 'nop' operator for the purposes of the enabler. These lines may not
    // be desirable for an edit engine, which would want to leave data in
    // tact, and would instead want to be in a seperate verification routine.
    if (hf->pIGen[uiCount].sfGenOper > endOper)
      hf->pIGen[uiCount].sfGenOper = nop;
  }
  for (uiCount = 0; uiCount < hf->awStructSize[instModList]; uiCount++) {
    tRIFF.SwapBytes(&hf->pIMod[uiCount].wModSrcOper);
    tRIFF.SwapBytes(&hf->pIMod[uiCount].wModDestOper);
    tRIFF.SwapBytes((WORD*)(&hf->pIMod[uiCount].shAmount));
  }
  //// Sample Headers ////
  for (uiCount = 0; uiCount < hf->awStructSize[sampHdr]; uiCount++) {


    tRIFF.SwapDWORD(&hf->pSHdr[uiCount].dwStart);
    tRIFF.SwapDWORD(&hf->pSHdr[uiCount].dwEnd);
    tRIFF.SwapDWORD(&hf->pSHdr[uiCount].dwStartloop);
    tRIFF.SwapDWORD(&hf->pSHdr[uiCount].dwEndloop);

    tRIFF.SwapDWORD(&hf->pSHdr[uiCount].dwSampleRate);

    //twoByteUnion temp;
    //temp.byVals.by1 = hf->pSHdr[uiCount].byOriginalKey;
    //temp.byVals.by0 = hf->pSHdr[uiCount].chFineCorrection;
    //tRIFF.SwapBytes(&temp.wVal);
    //hf->pSHdr[uiCount].byOriginalKey    = temp.byVals.by1;
    //hf->pSHdr[uiCount].chFineCorrection = temp.byVals.by0;

    tRIFF.SwapBytes(&hf->pSHdr[uiCount].wSampleLink);
    tRIFF.SwapBytes(&hf->pSHdr[uiCount].sfSampleType);

    if ((hf->pSHdr[uiCount].sfSampleType & romSample) == romSample)
      bROMSamples = TRUE;

    hf->pSHdr[uiCount].bSampleLoaded = FALSE;
  }

  /* It is here that we can do a valid WaveTable ID Check */

  pchIROMChunk = GetInfoSubCkVal(Irom);
  
  if ((bROMSamples == TRUE) && (pchIROMChunk == NULL))
  {
    /* ROM samples in a bank with no IROM chunk. Illegal bank. */
    SetError(EHC_WRONGWAVETABLE);
    delete hf;
    return NULL;
  }  

  if ((pchReqdWaveTable != NULL)  && 
      (pchIROMChunk     != NULL))
  { 

    /* If the CLIENT did NOT demand that loaded banks using ROM data
       match a particular WaveTable ID, bypass.
       If the ROM ID in the SoundFont bank is a null string, there is no 
       required WaveTable for the SF, so bypass.
       If neither is a null string, AND if the SoundFont bank DOES INDEED
       contain samples for a particular a WaveTable ROM, do a string 
       comparison between the CLIENT's ID and the SoundFont bank's ID.
    */
    if ((pchReqdWaveTable[0] != '\0')                   && 
	(pchIROMChunk[0]     != '\0')                   &&
        (bROMSamples == TRUE)                           &&
	(strcmp(pchReqdWaveTable, pchIROMChunk) != 0))
    {

      /* Non-matching wavetable ID when matching ID demanded, AND the
         Loaded SoundFont bank does contain samples expected to be
         in WaveTable ROM. This is an invalid bank for the current synth.
      */
      SetError(EHC_WRONGWAVETABLE);
      delete hf;
      return NULL;
    }
  }
  /* Return the hydraClass ptr. */

  return hf;

} // end ReadSFBFile

BYTEPTR sfReader::Allocate(SHORT        iHydraSymbol,
			   SHORT        iMemStructSize,
			   SHORT        iFileStructSize,
			   DWORD        dwChkSize,
	                   HydraClass   *hf)
//*************************************************************************
//
// Parameters: iHydraSymbol:    The SoundFont Data Structure symbol
//
//             iMemStructSize:  The size in bytes of memory required by
//                              a HydraFont head, (ie sizeof(whatever)
//
//             iFileStructSize: The size in bytes occupied in a file
//                              by a HydraFont head, 
//
//  Systems with different byte boundries will allocate memory from
//  the heap differently. Thus when allocating and accessing memory,
//  the iMemStructSize tells use the padded size of memory if applicable.
//
//***************************************************************************
{

sfVersion  *fileVersion; 
DWORD      rcnt; 
BYTEPTR    pData = NULL; 

WORD       mono =        monoSample; 


  tRIFF.SwapBytes(&mono);

  fileVersion = (sfVersion*)(infoCkValues[Ifil]);

  if ( iHydraSymbol == sampHdr )  {
     
    fileVersion = (sfVersion*)(infoCkValues[Ifil] );

    // get the count of how many sampleHdr elements in the array...

    hf->awStructSize[iHydraSymbol] = 
           (WORD) (dwChkSize /(DWORD) SAMPHDRV2_SIZE); // 46 bytes/element

    // now allocate our in memory representation... based on elements
    // times the inmemory size, (ie sizeof from the invocation ) 

    if ((pData = (BYTEPTR) Alloc((DWORD) hf->awStructSize[iHydraSymbol] * 
				       iMemStructSize)) == 0)   {
      SetError(EHC_ALLOCATE_PMEM_ERROR); 
      return (pData);
    }
    Memset( pData,0,hf->awStructSize[iHydraSymbol]*iMemStructSize);

    // now for however many elements we have lets do the reading...

    //
    // Element for element read of data, to serve as an example of a possible
    // issue with cross platform code. See comments below.
    //
    for(WORD curHdr=0; curHdr < hf->awStructSize[iHydraSymbol]; curHdr++) { 

        rcnt = 0; 

        rcnt += tRIFF.RIFFRead((VOIDPTR)
                               &((sfSampleHdr*)pData)[curHdr].achSampleName, 
				             1, SAMPLENAMESIZE);

        rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].dwStart,       
	                                     1, sizeof(DWORD));

        rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].dwEnd, 
	 	 		             1, sizeof(DWORD));

        rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].dwStartloop, 
 					     1, sizeof(DWORD));

        rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].dwEndloop,  
 					     1, sizeof(DWORD));

        rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].dwSampleRate,
                                               1, sizeof(DWORD));

	rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].byOriginalKey,
					       1, sizeof(BYTE));

	rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].chFineCorrection,
					       1, sizeof(CHAR));

	rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].wSampleLink,
                                               1, sizeof(WORD));

        rcnt += tRIFF.RIFFRead( &((sfSampleHdr*)pData)[curHdr].sfSampleType,
                                               1, sizeof(WORD));

        if( rcnt != SAMPHDRV2_SIZE) { 
#ifdef DEBUG
	  cout<<"%Allocate-E-error reading sampleHdr V2 "<<endl; 
#endif
          Dealloc(pData);
          SetError(SF_INVALIDBANK);
          return (NULL);
        }
     }// end for all elements
  }  // do sampleHdr reading, 

  else { 

    hf->awStructSize[iHydraSymbol] = (WORD) (dwChkSize /
					  (DWORD) iFileStructSize);
    if ((pData = (BYTEPTR) Alloc((DWORD) hf->awStructSize[iHydraSymbol] * 
				       iMemStructSize)) == 0)
    {
      SetError(EHC_ALLOCATE_PMEM_ERROR); 
      return (pData);
    }

    DWORD dwNdx;


    for (WORD wNdx= 0; wNdx< hf->awStructSize[iHydraSymbol]; wNdx++)
    {
      dwNdx = (DWORD)((DWORD)wNdx* (DWORD)iMemStructSize);

      //
      // NOTE: ANSI makes NO specification about how elements in a 
      //       data structure is ALIGNED within that data structure.
      //       Although a byte-for-byte copy from the file image into
      //       the memory image of the Data Structure like what is happening
      //       below works with many major commercial compilers for Intel
      //       Motorola, and Sparc microprocessors under DOS, Windows,
      //       Mac System 7, SunOS 4.3, and other major operating systems there 
      //       are no guarantees it will work EVERYWHERE. 
      //
      //       This READ may need to be re-written to stuff all ELEMENTS
      //       of each Data Structure INDIVIDUALLY to guarantee alignment!
      //       (See how the SampleHeader reads are done above)
      //
      //       On the OTHER hand...
      //       A possible OPTIMIZATION is that if you are on a 16 bit 
      //       bounded system, AND you are NOT reading the defined
      //       INST and SHDR structures (iHydraSymbol == inst OR sampHdr)
      //       you can do large reads spanning multiple data structures.
      //       and get all data in one shot...
      //
      //       IE Kill the 'for' loop and do:
      //           tRIFF.RIFFRead((VOIDPTR)pData, 1, 
      //                          hf->awStructSize[iHydraSymbol] * 
      //                          iMemStructSize);
      //
      //       To do this for ALL data structures, remove the RefCount and
      //       sampleLoaded fields from the definitons of the inst and 
      //       sampHdr structs FIRST!
      //
      if (0 == tRIFF.RIFFRead((VOIDPTR)(&pData[dwNdx]),
			      1,
			      iFileStructSize))
      {
        Dealloc(pData);
        pData = NULL;
        SetError(EHC_READFILEERROR); 
	break;
      }
    }  // end to read all elements
  } // end to new else do normal thing.

  return (pData);

}// end Allocate

SHORT sfReader::storeInfoCks(void)
/*
===============================================================================
* 
*  Implementation Notes: 
*   Called from ReadSFBData(), this routine is responsible for allocating and 
* storing the info sub-chunks values that are preset in the currently loaded
* SoundFont Bank file, (if any). Many of these sub-chunks are optional, these
* values will remain NULL. These values are stored in the static file global
* object "infoCkValues" which is an array with extent infoSubCkCount of 
* CHAR pointers. 
*
* We just iterate throught the list of defined 'tokens' asking to 'findCk'
* from RIFF, and if found, we allocate space for it and place it in the 
* list, if the value is one of the exceptional non-string values, we 
* cast it into a string anyway. The caller of getInfoCkVal() will utilize
* the size out value to correctly cast the sucker back into whatever struct
* type it really wants to be.
*
===============================================================================
*/
{
WORD curToken; 
LONG stat = 0; 
sfVersion *ver;

   /*  range is Ifil..Icmt */ 

   resetInfoCkValues();   /* Make sure we are playing with a clean slate */

   for(curToken = Ifil; curToken <= Icmt; curToken++) { 

       if( tRIFF.FindCk(tRIFF.GetRIFFToken(curToken)) == SUCCESS ) {

	 infoCkValues[curToken ] = new CHAR[ tRIFF.GetCkSize() + 1];

	 if (infoCkValues[curToken] == NULL)
	 {
	   SetError(EHC_ALLOCATE_PMEM_ERROR);
	   //resetInfoCkValues();
	   return EHC_ALLOCATE_PMEM_ERROR;
	 }

	 memset(infoCkValues[curToken], 0, (size_t)(tRIFF.GetCkSize() +1));

	 stat = tRIFF.RIFFRead(infoCkValues[curToken], 
			       1, (size_t) tRIFF.GetCkSize());

	 if ( (curToken == Ifil) || ( curToken == Iver )) { /*its a struct...*/

	   ver = (sfVersion*)infoCkValues[curToken];
	   tRIFF.SwapBytes(&ver->wMajor);
	   tRIFF.SwapBytes(&ver->wMinor);
	
	  #ifdef DEBUG
  	    cout <<"%StoreInfoCks-I-saw token "<< 
	     tRIFF.GetRIFFToken(curToken)
	       <<" and stored the value  "
	       << ver->wMajor<< "."<< ver->wMinor << endl;
	  #endif
	 }
	 else { /* its just a string... */ 

	 #ifdef DEBUG
	   cout<<"%StoreInfoCks-I-saw token "<<tRIFF.GetRIFFToken(curToken)
	      <<" and stored the value "<< infoCkValues[curToken] << endl;
	 #endif
	}       
     }//
     else { // we didn't find the current chunk, but wait! is it the ifil?
       if(curToken == Ifil) { // check if the uppercase equiv it present...
         if( tRIFF.FindCk("IFIL") == SUCCESS ) {
  	   infoCkValues[curToken ] = new CHAR[ tRIFF.GetCkSize() + 1];
  	   stat = tRIFF.RIFFRead(infoCkValues[curToken], 
			         1, (SHORT) tRIFF.GetCkSize());
	   ver = (sfVersion*)infoCkValues[curToken];
	   tRIFF.SwapBytes(&ver->wMajor);
	   tRIFF.SwapBytes(&ver->wMinor);
         }
       }
     }//end else

   }// end for all chunks
  
  /*  ok we make sure there is some kind of name for the bank, if there was
   *  was no INAM chunk found, we'll put in a default of SoundFont Bank  
   */ 

  if(infoCkValues[Inam] == NULL) {

     if ((infoCkValues[Inam] = new CHAR[15]) == NULL)
     {
       //resetInfoCkValues();
       SetError(EHC_ALLOCATE_PMEM_ERROR);
       return EHC_ALLOCATE_PMEM_ERROR;
     } 
     strcpy(infoCkValues[Inam],"SoundFont Bank");
  }

 return SHORT(stat);     
}/*  end storeInfoCks; */ 



CHAR* sfReader::GetInfoSubCkVal(RiffCkTokenType token)
/*===========================================================================
* 
* Implementation Notes: 
*
*   Simply return the value stored a position indicated by the 
* value of token. The only check is to see if the given tokens
* value is between Ifil and Icmt inclusive...
* If the particular token values you asked for is null, the function
* returns NULL. -be aware-
*
===============================================================================
*/
{
  if(( token < Ifil)||(token > Icmt)) { 
   SetError(EHC_NOTINFOTOKEN);       /* out of range, not an infosubchunk */
   return NULL;
   }
  return infoCkValues[token];
} /*  end GetSubkVal */ 


void sfReader::resetInfoCkValues(void)

/*****************************************************************************
*
* Implementation Notes:
*
*   This routine must deallocate last infosubChunk values if needed, 
* this is to say, if we are going read _another_ file with this same
* instance of the sfReader class, we had better deallocate and 'erase'
* the previous files info sub-chunk values. If a client has erronously 
* copied our pointers, rather than make explict deep copies of the data, 
* they will find out the error of their ways upon dereferencing bogosity.
*
******************************************************************************
*/
{
  for(LONG i=0; i<CkCount; i++) 
  {
    if (infoCkValues[i] != NULL) 
    {
      #ifdef DEBUG
       cout <<"%ResetInfoCks-I-saw deleting info chunk "<< i <<" which was ";
       if ( (i == Ifil) || ( i == Iver ))
       { /*its a struct...*/

           sfVersion *ver;
	   ver = (sfVersion*)infoCkValues[i];
	   tRIFF.SwapBytes(&ver->wMajor);
	   tRIFF.SwapBytes(&ver->wMinor);
	   cout << ver->wMajor<< "."<< ver->wMinor << endl;
       }
       else
	   cout << infoCkValues[i] << endl;
      #endif
      delete [] infoCkValues[i];
      infoCkValues[i] = NULL;
    }
  }

}/* end resetInfoCkValues; */ 

/*****************************************************************************
* 
* sfReader::SetupToFillSampleBuckets
*
* Implementation Notes:  
*
*  The reader is being called upon for the sample data. The data storage
* medium must be reopened (the reader should have the keys required to do 
* so) and the variables for the sample dump routine must be set up properly.
*
******************************************************************************
*/
void sfReader::
SetupToFillSampleBuckets(void)
{
  EMUSTAT emuStat = SUCCESS;
  SetError(SUCCESS);

  if (bySFStorageStatus == SFR_CLOSED)
  {
  switch (bySFDataLoc)
  {
    case SFR_ONDISK:
      emuStat = tRIFF.OpenRIFF(pathName);
      break;
    case SFR_ONMACDISK:
      #ifdef USE_MACINTOSH
      emuStat = tRIFF.OpenRIFF(pSpecifier);
      break;
      #endif // Yes, no USE_MACINTOSH has this case go to default
    default:
      SetError(EHC_NOFONT);
      return;
  }
  }

  if (emuStat != SUCCESS)
  {
    SetError(EHC_OPENFILEERROR);
    return;
  }

  if (tRIFF.FindCk(tRIFF.GetRIFFToken(Smpl)) != SUCCESS )
  {
    SetError(EHC_RIFFERROR);
    bySFStorageStatus = SFR_CLOSED;
    tRIFF.CloseRIFF();
  }

  dwSampleSize = tRIFF.GetCkSize();
  dwSampleOffset = tRIFF.RIFFTellAbs(); // defaults to RIFFTell for all but
					// SoundROM data.
  dwSampleBytesCollected = 0L;
  return;
}

/*****************************************************************************
* 
* sfReader::SetupToFillSampleBuckets
*
* Implementation Notes:  
*
*  The reader is being called upon for SOME OF the sample data. The data storage
* medium must be reopened (the reader should have the keys required to do 
* so) and the variables for the sample dump routine must be set up properly.
*
* Note what is passed in is the start and end points IN WORDS and RELATIVE
* to the FIRST Sample Start point. This way, the dwStart and dwEnd parameters
* pass in ARE the SoundFont Sample Header dwStart and dwEnd values.
* 
* IE: SetupToFillSampleBuckets(pHF->pSHdr[sample].dwStart, 
*                              pHF->pSHdr[sample].dwEnd);
*
******************************************************************************
*/
void sfReader::
SetupToFillSampleBuckets(DWORD dwStart, DWORD dwEnd)
{
  //EMUSTAT emuStat = SUCCESS;
  SetError(SUCCESS);

  SetupToFillSampleBuckets();
  if ( GetError() !=  SUCCESS) return;

  dwSampleSize = (dwEnd - dwStart) << 1;
  tRIFF.RIFFSeek(dwSampleOffset+(dwStart<<1), SEEK_SET);
  dwSampleOffset = tRIFF.RIFFTellAbs();
}

/*****************************************************************************
*
* sfReader::FillSampleBucket
*
* Implementation Notes:
*
*  The reader is being called upon for the sample data. This fills the
* buffer with the amount of data specified by the *pdwSize. The actual
* amount of data (in BYTES) filled is stuffed into *pdwSize in this routine.
* If all data has been collected, *pdwSize is set to 0.
*
*  If SetupToFillSampleBuckets() was never called, the variables used would
* indicate that all data has already been collected. This allows
* this routine to exit smoothly if called inappropriately.
*
*  Note there are 3 sizes to consider on any read:
* 1. dwThisBucketSize: The size in bytes of the bucket requested by the client.
* 2. dwSampleBytesCollected: The size in bytes of the total sample data space
*                            of this reader's SoundFont
* 3. MAX_READ_SIZE: The size in bytes of the largest possible data read.
*
* So the algorythm tries to fill up the bucket (specified by dwThisBucketSize)
* in blocks of MAX_READ_SIZE until either the bucket is full or until
* the total number of bytes collected equals the total sample data space of
* the SoundFont. When the latter condition is met, the SoundFont data storage
* medium is closed.
*
******************************************************************************
*/
void sfReader::
FillSampleBucket(BYTE *pbyBucket, DWORD * pdwSize)
{

  SHORT iReadSize;
  SHORT iReadSizeAct;
  WORD uiNdx = 0;//, uiCount;
  DWORD dwThisBucketSize = *pdwSize;

  if (dwSampleBytesCollected >= dwSampleSize)
  {
    *pdwSize = 0;
    return;
  }

  if ((dwSampleSize - dwSampleBytesCollected) < dwThisBucketSize)
       dwThisBucketSize = dwSampleSize - dwSampleBytesCollected;

  *pdwSize = 0;

  for (; dwThisBucketSize > 0; dwThisBucketSize -= iReadSizeAct, uiNdx++)
  {

    iReadSize = (dwThisBucketSize > MAX_READ_SIZE) ? MAX_READ_SIZE:
							 (SHORT)dwThisBucketSize;

    iReadSizeAct =
      tRIFF.RIFFRead(&pbyBucket[uiNdx*MAX_READ_SIZE], iReadSize, 1);

    if (iReadSizeAct != 1)
      return;

    iReadSizeAct = iReadSize;
    #ifdef __BYTE_INCOHERENT
    //*******************************************
    // Yes, samples need to be byte-swapped too!
    //*******************************************
    for (uiCount = 0; uiCount < iReadSizeAct; uiCount +=2)
    {
      tRIFF.SwapBytes((WORD *)&pbyBucket[uiCount]);
    }
    #endif

    *pdwSize += iReadSize;

    dwSampleBytesCollected += iReadSize;

    if (dwSampleBytesCollected >= dwSampleSize)
    {
      resetSampleCollector();
      tRIFF.CloseRIFF();
      bySFStorageStatus = SFR_CLOSED;
      break;
    }
  }
}

/*****************************************************************************
* 
* sfReader::IsValid
*
* Implementation Notes:  
*
*  A simple BOOLean function to query whether or not the reader has actually
* read something yet.
*
******************************************************************************
*/
BOOL sfReader::
IsValid(void)
{
  if (bySFDataLoc == SFR_NO_DATA) return(FALSE);
  return(TRUE);
}

/*****************************************************************************
* 
* sfReader::resetSampleCollector
*
* Implementation Notes:  
*
*  Resets the sample data collection variables
*
******************************************************************************
*/
void sfReader::
resetSampleCollector(void)
{
  dwSampleSize = 0;
  dwSampleBytesCollected = 0;
  dwSampleOffset = 0;
}

