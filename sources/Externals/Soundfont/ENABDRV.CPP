     /*********************************************************************
     
     enabdrv.cpp
     
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
#include <stdlib.h>
#include <stdio.h>
#include <iostream> 
#include <fstream>
#include "Application/Utils/assert.h"

#include "DATATYPE.H"

#include "ENAB.H"

using namespace std ;


//-****************************************************************************
// @(#)enabdrive.cpp	1.2 13:56:24 5/31/95 13:56:25 
//
// Description: 
//
//   This module defines an interactive test utilty used to 'drive' the 
// enabler. A main menu is presented and selections are made to test various
// functions offered by the enabler.
// 
//   Options exist to display all preset (SoundFont Objects) names, and their
// MIDI Bank and MIDI Preset number assignments, all sample names and 
// address statistics, as well as "navigate" a preset as a MIDI note-on event
// would. In additon, you may dump the private implmentation, (such as it is)
// of the currently loaded banks array. Currently the enabler allows up to 
// 3 (three) SoundFont Banks to be loaded at once, See enab.h defined constant:
//
//    #define MAXLOADEDBANKS 3
//
// if more banks are desired...
// 
// Usage: 
//    $ enabdrive [pagesize]
//   The optional argument pagesize defines the number of lines of output 
// generated before a pagebreak -----[more n|q] --- is generated. 
// 'n' (or anythng _Not_ a 'q') for next page, 'q' to return to menu. 
//
// Current default value: 40; 
// 
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May '95      Initial Creation, Enabler
//-***************************************************************************

typedef enum cmd {
    quit,   
    menu, 
    load, 
    unload, 
    navigate,
    getPresets,
    getSampleHdrs,
    dumpBankNodesArray

}mainMenuCmdType;

WORD pageSize = 40; 
static CHAR msg[80];


const CHAR *getErrorString(enaErrors theEnablerError)
//*****************************************************************************
// 
// Implementation Notes: 
//
//   Just get the input...
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     June   94     initial creation, unit driver
//****************************************************************************
{

  switch (theEnablerError) {
 
  case enaSUCCESS:
    return "Successful.\n\0";
    //break;

  case enaLOADERROR:
    return "There was a bank load error.\n\0"; 
    //break;

  case enaENOSUCHBANK: 
    return "The selected bank index does not exist.\n\0";
    //break;

  case enaENOSUCHPRESET: 
    return "The selected preset index does not exist.\n\0";
    //break;

  case enaEBANKTABLEFULL:
    return "The table for Multiple SF bank load is full.\n\0";
    //break;

  case enaWBANKNOTLOADED:
    return "The selected bank is not loaded.\n\0";
    //break;

  case enaEBANKALREADYLOADED:
    return "The selected bank is already loaded.\n\0";
    //break;

  case enaVERIFYERROR:
    return "The selected bank has INTERNAL data structure errors.\n\0";
    //break;

  default:
    sprintf(msg, "Error type %d\n.", (WORD)theEnablerError);
    return (const CHAR *)msg;
    //break;
  }
} // end getErrorString

CHAR *getInput()
//*****************************************************************************
// 
// Implementation Notes: 
//
//   Just get the input...
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     June   94     initial creation, unit driver
//****************************************************************************
{
CHAR *input; 

   input = new CHAR[PRESETNAMESIZE];
   memset(input,0,PRESETNAMESIZE);
   input = gets(input);
   n_assert(cin.bad() == 0 ); 
        
  //    cout << " getInput is returning" << input << endl; 
   
 return input;

}// end getinput


WORD getCmd()
//*****************************************************************************
// 
// Implemetation Notes: 
//
//   Just prompt for and obtain the next cmd. 
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     June   94     initial creation, unit driver
//
//****************************************************************************
{
SHORT next = 0;
 
CHAR *s = new CHAR[50];

  cout << "Next?> " ; 
  cin >> next; 
  if( cin.fail() ) {
      cout << "cin.fail " << cin.fail() << endl; 
      cin.clear();
      exit(-1);  
  }
  return (next);

}// end getCmd; 

void doLoadBank()
//*****************************************************************************
// 
// Implementation Notes: 
//
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
sfBankID  bankID = -1; 
CHAR *bankFileName; 

  cout<<"LoadBank-I-Please enter bank file name "; cout.flush(); 
  bankFileName = getInput();  
  cout<<endl;

  cout<<"...Attempting to load " << bankFileName <<"..."; 
  cout.flush();
   
  bankID  = sfReadSFBFile(bankFileName); 

  if(bankID != -1) { 
    cout <<endl; 
    cout <<"...Bank Loaded, ID returned : " << bankID <<endl<<endl; 
  }
  else { 
    cout << "Failed: " << getErrorString(sfGetError()) <<endl<<endl; 
    if(sfGetError() == enaEBANKTABLEFULL) { 
      cout<<"...Load failed, enabler table full, only " << MAXLOADEDBANKS
          <<" banks allowed concurrently"<<endl; 
    }
    //else { 
      //cout<<"...with omega value: " << sfGetError() << endl; 
    //}
  }

return; 
}// end doLoadBank;

void doUnloadBank()
//*****************************************************************************
// 
// Implementation Notes: 
//
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
SHORT       stat    = enaSUCCESS; 
SHORT       bankID  = -1; 

  cout<<"doUnloadBank-I-Please enter the bankID of the bank to unload: "; 
  cout.flush(); 
  cin >> bankID; 
  cout<<endl;

  cout<<"...Attempting to unload bankID " << bankID<< "..."; 
  cout.flush(); 
  
  stat = sfUnloadSFBank(bankID); 

  if( stat != enaSUCCESS) { 
    if(sfGetError() == enaENOSUCHBANK) { 
      cout<<"...No such bank with ID " << bankID << endl; 
    }  
    else { 
      cout<<"Error unloading bankID of " << bankID << endl; 
    }
  }
  else { 
    if(sfGetError() == enaWBANKNOTLOADED) { 
      cout<<"...no bank loaded with an ID of " << bankID << endl; 
    }
    else { 
     cout<<"Bank Unloaded " <<endl; 
   }
  }
  cout<<endl; 
  return; 
	
}// end doUnloadBank; 


void dumpSfDataToFile(sfData *vec, WORD cnt )
//*****************************************************************************
// 
// Implementation Notes: 
//
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
//****************************************************************************
{
WORD lineCount = 0; 
CHAR answer   = 'n'; 
CHAR filename[80];
CHAR data[80];
FILE *outFile;
  
  cout << "Filename? ";
  cout.flush();
  cin >> filename;
  cout << endl;

  if ((outFile = fopen(filename, "w\0")) == NULL)
  {
    cout << "Cannot open " << filename << "." << endl;
    return;
  }

  for(WORD curElem = 0; curElem < cnt; curElem++ ) { 

    sprintf(data, "\n\n--------- Vector # %d --------------\n\n", curElem);
    fputs(data, outFile);

    sprintf(data, "dwStart                  %lu\tsamples\n", 
                                                   vec[curElem].dwStart);
    fputs(data, outFile);
    sprintf(data, "dwEnd                    %lu\tsamples\n",
                                                   vec[curElem].dwEnd);
    fputs(data, outFile);
    sprintf(data, "dwStartLoop              %lu\tsamples\n",
                                                   vec[curElem].dwStartloop);
    fputs(data, outFile);
    sprintf(data, "dwEndloop                %lu\tsamples\n",   
                                               vec[curElem].dwEndloop);
    fputs(data, outFile);
    sprintf(data, "dwSampleRate             %lu\tHz\n",   
                                               vec[curElem].dwSampleRate);
    fputs(data, outFile);
    sprintf(data, "OriginalKey              %d\t\n",
                                   (WORD)(vec[curElem].shOrigKeyAndCorr)/256);
    fputs(data, outFile);

    sprintf(data, "Correction               %d\tcents\n",
                          (SHORT)((CHAR)(vec[curElem].shOrigKeyAndCorr&0xFF)));
    fputs(data, outFile);


    sprintf(data, "shModLfoToPitch          %d\tcents/max excursion\n",   
                                               vec[curElem].shModLfoToPitch);  
    fputs(data, outFile);
    sprintf(data, "shVibLfoToPitch          %d\tcents/max excursion\n",       
                                               vec[curElem].shVibLfoToPitch);
    fputs(data, outFile);

    sprintf(data, "shModEnvToPitch          %d\tcents/max excursion\n",       
                                               vec[curElem].shModEnvToPitch);
    fputs(data, outFile);

    sprintf(data, "shInitialFilterFc        %d\tcents above 8.176 Hz\n",   
                                               vec[curElem].shInitialFilterFc);
    fputs(data, outFile);

    sprintf(data, "shInitialFilterQ         %d\tcB above DC level\n",
                                               vec[curElem].shInitialFilterQ);
    fputs(data, outFile);

    sprintf(data, "shModLfoToFilterFc       %d\tcents/max excursion\n",       
                                               vec[curElem].shModLfoToFilterFc);
    fputs(data, outFile);

    sprintf(data, "shModEnvToFilterFc       %d\tcents/max excursion\n",       
                                               vec[curElem].shModEnvToFilterFc);
    fputs(data, outFile);

    sprintf(data, "shModLfoToVolume         %d\tcB/max excursion\n",          
                                               vec[curElem].shModLfoToVolume  );
    fputs(data, outFile);

    sprintf(data, "shChorusEffectsSend      %d\tunits of 0.1%%\n",          
                                             vec[curElem].shChorusEffectsSend );
    fputs(data, outFile);

    sprintf(data, "shReverbEffectsSend      %d\tunits of 0.1%%\n",          
                                            vec[curElem].shReverbEffectsSend );
    fputs(data, outFile);

    sprintf(data, "shPanEffectsSend         %d\tunits of 0.1%% full right\n",
                                               vec[curElem].shPanEffectsSend);
    fputs(data, outFile);

    sprintf(data, "shDelayModLfo            %d\tTimeCents above 1 second\n",
                                               vec[curElem].shDelayModLfo    );
    fputs(data, outFile);

    sprintf(data, "shFreqModLfo             %d\tcents above 8.176 Hz\n",  
                                               vec[curElem].shFreqModLfo     );
    fputs(data, outFile);

    sprintf(data, "shDelayVibLfo            %d\tTimeCents above 1 second\n",
                                               vec[curElem].shDelayVibLfo    );
    fputs(data, outFile);

    sprintf(data, "shFreqVibLfo             %d\tcents above 8.176 Hz\n",  
                                               vec[curElem].shFreqVibLfo     );
    fputs(data, outFile);

    sprintf(data, "shDelayModEnv            %d\tTimeCents above 1 second\n",
                                               vec[curElem].shDelayModEnv    );
    fputs(data, outFile);

    sprintf(data, "shAttackModEnv           %d\tTimeCents above 1 second\n",
                                               vec[curElem].shAttackModEnv   );
    fputs(data, outFile);

    sprintf(data, "shHoldModEnv             %d\tTimeCents above 1 second\n",
                                               vec[curElem].shHoldModEnv     );
    fputs(data, outFile);

    sprintf(data, "shDecayModEnv            %d\tTimeCents above 1 second\n",
                                               vec[curElem].shDecayModEnv    );
    fputs(data, outFile);

    sprintf(data, "shSustainModEnv          %d\tcB Attenuation\n",
                                               vec[curElem].shSustainModEnv  );
    fputs(data, outFile);

    sprintf(data, "shReleaseModEnv          %d\tTimeCents above 1 second\n",
                                               vec[curElem].shReleaseModEnv  );
    fputs(data, outFile);

    sprintf(data, "shAutoHoldModEnv         %d\tTimeCents / KeyNum\n",    
                                               vec[curElem].shAutoHoldModEnv );
    fputs(data, outFile);

    sprintf(data, "shAutoDecayModEnv        %d\tTimeCents / KeyNum\n",   
                                               vec[curElem].shAutoDecayModEnv );
    fputs(data, outFile);

    sprintf(data, "shDelayVolEnv            %d\tTimeCents above 1 second\n",
                                               vec[curElem].shDelayVolEnv    );
    fputs(data, outFile);

    sprintf(data, "shAttackVolEnv           %d\tTimeCents above 1 second\n",
                                               vec[curElem].shAttackVolEnv   );
    fputs(data, outFile);

    sprintf(data, "shHoldVolEnv             %d\tTimeCents above 1 second\n",
                                               vec[curElem].shHoldVolEnv     );
    fputs(data, outFile);

    sprintf(data, "shDecayVolEnv            %d\tTimeCents above 1 second\n",
                                               vec[curElem].shDecayVolEnv    );
    fputs(data, outFile);

    sprintf(data, "shSustainVolEnv          %d\tcB Attenuation\n",
                                               vec[curElem].shSustainVolEnv  );
    fputs(data, outFile);

    sprintf(data, "shReleaseVolEnv          %d\tTimeCents above 1 second\n",
                                               vec[curElem].shReleaseVolEnv  );
    fputs(data, outFile);

    sprintf(data, "shAutoHoldVolEnv         %d\tTimeCents / KeyNum\n",    
                                               vec[curElem].shAutoHoldVolEnv );
    fputs(data, outFile);


    sprintf(data, "shAutoDecayVolEnv        %d\tTimeCents / KeyNum\n",    
                                               vec[curElem].shAutoDecayVolEnv);
    fputs(data, outFile);

    sprintf(data, "shKeynum                 %d\t\n",                      
                                               vec[curElem].shKeynum       );
    fputs(data, outFile);

    sprintf(data, "shVelocity               %d\t\n",                      
                                               vec[curElem].shVelocity     );
    fputs(data, outFile);

    sprintf(data, "shInstVol                %d\tcB of Attenuation\n",     
                                               vec[curElem].shInstVol      );
    fputs(data, outFile);

    sprintf(data, "shCoarseTune             %d\tSemitones\n",             
                                               vec[curElem].shCoarseTune   );
    fputs(data, outFile);

    sprintf(data, "shFineTune               %d\tcents\n",                 
                                               vec[curElem].shFineTune     );
    fputs(data, outFile);

    sprintf(data, "shSampleModes            %d\t\n",                      
                                               vec[curElem].shSampleModes  );
    fputs(data, outFile);

    if (vec[curElem].shSampleModes & LINKED)
    {
      sprintf(data, "shSampleLink             %d\t\n",                      
                                               vec[curElem].shSampleLink  );
      fputs(data, outFile);
    }

    sprintf(data, "shKeyExclusiveClass      %d\t\n",                      
                                            vec[curElem].shKeyExclusiveClass);
    fputs(data, outFile);

  }// end all elements

  fclose(outFile);

}// end dumpSfData; 

void dumpSfData(sfData *vec, WORD cnt )
//*****************************************************************************
// 
// Implementation Notes: 
//
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
WORD lineCount = 0; 
CHAR answer   = 'n'; 
  
  for(WORD curElem = 0; curElem < cnt; curElem++ ) { 

    cout<<"--------- Vector # "<<curElem<<"--------------" <<endl<<endl;

    cout<<"dwStart                  " << vec[curElem].dwStart << 
          "	samples"              << endl; 
    cout<<"dwEnd                    " << vec[curElem].dwEnd  << 
          "	samples"                  << endl; 
    cout<<"dwStartLoop              " << vec[curElem].dwStartloop << 
          "	samples"                  << endl; 
    cout<<"dwEndloop                " << vec[curElem].dwEndloop << 
          "	samples"                  << endl; 
    cout<<"dwSampleRate             " << vec[curElem].dwSampleRate << 
          "	Hz"                  << endl; 
    cout<<"OriginalKey              " << 
                                   (WORD)(vec[curElem].shOrigKeyAndCorr)/256 << 
          " "                         << endl; 
    cout<<"Correction               " << 
                                   (SHORT)((CHAR)(vec[curElem].shOrigKeyAndCorr&0xFF))<< 
          "	cents"                         << endl; 


    cout<<" more ----- [n|q] :"; 
    cout.flush(); 
    cin >> answer; 
    if( answer == 'q') { 
	break; 
    }
    cout<<"shModLfoToPitch          " << vec[curElem].shModLfoToPitch   << 
          "	cents/max excursion"         << endl; 
    cout<<"shVibLfoToPitch          " << vec[curElem].shVibLfoToPitch << 
          "	cents/max excursion"         << endl; 
    cout<<"shModEnvToPitch          " << vec[curElem].shModEnvToPitch << 
          "	cents/max excursion"         << endl; 
    cout<<"shInitialFilterFc        " << vec[curElem].shInitialFilterFc << 
          "	cents above 8.176 Hz"     << endl; 
    cout<<"shInitialFilterQ         " << vec[curElem].shInitialFilterQ << 
          "	cB above DC level"        << endl; 
    cout<<"shModLfoToFilterFc       " << vec[curElem].shModLfoToFilterFc << 
          "	cents/max excursion"         << endl; 
    cout<<"shModEnvToFilterFc       " << vec[curElem].shModEnvToFilterFc << 
          "	cents/max excursion"         << endl; 

    cout<<"shModLfoToVolume         " << vec[curElem].shModLfoToVolume   << 
          "	cB/max excursion"            << endl; 
    cout<<"shChorusEffectsSend      " << vec[curElem].shChorusEffectsSend  << 
          "	units of 0.1%"            << endl; 
    cout<<"shReverbEffectsSend      " << vec[curElem].shReverbEffectsSend  << 
          "	units of 0.1%"            << endl; 
    cout<<"shPanEffectsSend         " << vec[curElem].shPanEffectsSend <<
          "	units of 0.1% full right" << endl; 

    cout<<" more ----- [n|q] :"; 
    cout.flush(); 
    cin >> answer; 
    if( answer == 'q') { 
	break; 
    }
    cout<<"shDelayModLfo            " << vec[curElem].shDelayModLfo     << 
          "	TimeCents above 1 second" << endl; 
    cout<<"shFreqModLfo             " << vec[curElem].shFreqModLfo      << 
          "	cents above 8.176 Hz"     << endl; 
    cout<<"shDelayVibLfo            " << vec[curElem].shDelayVibLfo     << 
          "	TimeCents above 1 second" << endl; 
    cout<<"shFreqVibLfo             " << vec[curElem].shFreqVibLfo      << 
          "	cents above 8.176 Hz"     << endl; 
    cout<<"shDelayModEnv            " << vec[curElem].shDelayModEnv     <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shAttackModEnv           " << vec[curElem].shAttackModEnv    <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shHoldModEnv             " << vec[curElem].shHoldModEnv      <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shDecayModEnv            " << vec[curElem].shDecayModEnv     <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shSustainModEnv          " << vec[curElem].shSustainModEnv   <<
          "	cB Attenuation" << endl; 
    cout<<"shReleaseModEnv          " << vec[curElem].shReleaseModEnv   <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shAutoHoldModEnv         " << vec[curElem].shAutoHoldModEnv  <<
          "	TimeCents / KeyNum"       << endl; 
    cout<<"shAutoDecayModEnv        " << vec[curElem].shAutoDecayModEnv <<
          "	TimeCents / KeyNum"       << endl; 
    cout<<"shDelayVolEnv            " << vec[curElem].shDelayVolEnv     <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shAttackVolEnv           " << vec[curElem].shAttackVolEnv    <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shHoldVolEnv             " << vec[curElem].shHoldVolEnv      <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shDecayVolEnv            " << vec[curElem].shDecayVolEnv     <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shSustainVolEnv          " << vec[curElem].shSustainVolEnv   <<
          "	cB Attenuation" << endl; 
    cout<<"shReleaseVolEnv          " << vec[curElem].shReleaseVolEnv   <<
          "	TimeCents above 1 second" << endl; 
    cout<<"shAutoHoldVolEnv         " << vec[curElem].shAutoHoldVolEnv  <<
          "	TimeCents / KeyNum"       << endl; 

    cout<<" more ----- [n|q] :"; 
    cout.flush(); 
    cin >> answer; 
    if( answer == 'q') { 
	break; 
    }
    cout<<"shAutoDecayVolEnv        " << vec[curElem].shAutoDecayVolEnv <<
          "	TimeCents / KeyNum"       << endl; 
    cout<<"shKeynum                 " << vec[curElem].shKeynum        <<
          " "                         << endl; 
    cout<<"shVelocity               " << vec[curElem].shVelocity      <<
          " "                         << endl; 
    cout<<"shInstVol                " << vec[curElem].shInstVol       <<
          "	cB of Attenuation"        << endl; 
    cout<<"shCoarseTune             " << vec[curElem].shCoarseTune    <<
          "	Semitones"                << endl; 
    cout<<"shFineTune               " << vec[curElem].shFineTune      <<
          "	cents"                    << endl; 
    cout<<"shSampleModes            " << vec[curElem].shSampleModes   <<
          " "                         << endl; 
    if (vec[curElem].shSampleModes & LINKED)
    {
      cout<<"shSampleLink             " << vec[curElem].shSampleLink    <<
            " "                         << endl; 
    }
    cout<<"shKeyExclusiveClass      " << vec[curElem].shKeyExclusiveClass <<
          " "                         << endl; 
    cout.flush();

  }// end all elements

}// end dumpSfData; 

void doNav()
//*****************************************************************************
// 
// Implementation Notes: 
//
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
sfBankID sfBank;
WORD MidiBank; 
WORD MidiPreset; 
WORD MidiKeyNum;
WORD Velocity; 
CHAR answer; 

sfData *retVec  = NULL; 
WORD   cnt      = 0; 

   cout<<"doNav-I-Please enter sfBankID     > " ;  cout.flush();
   cin >> sfBank; 
   cout<< endl; 
   cout<<"doNav-I-Please enter MIDI bank    > " ; cout.flush();
   cin >> MidiBank; 
   cout<< endl; 
   cout<<"doNav-I-Please enter MIDI preset  > " ; cout.flush();
   cin >> MidiPreset; 
   cout<< endl;
   cout<<"doNav-I-Please enter MIDI note    > " ; cout.flush();
   cin >> MidiKeyNum; 
   cout<<endl; 
   cout<<"doNav-I-Please enter key velocity > " ; cout.flush();
   cin >> Velocity; 
   cout<<endl; 

   retVec = sfNav( sfBank, MidiBank, MidiPreset, MidiKeyNum, Velocity, &cnt);
  
   if(retVec != NULL) { 
     cout<<"...Obtained " << cnt <<" sfData elements, want to see em? [y|n]";
     cout.flush(); 
     cin >> answer; 
     cout << endl;
     if ( answer == 'y' ) { 
       dumpSfData(retVec, cnt);	  
       }
     cout<<"Dump to file? [y|n]";
     cout.flush(); 
     cin >> answer; 
     cout << endl;
     if ( answer == 'y' ) { 
       dumpSfDataToFile(retVec, cnt);	  
       }

   }       // end if retVec non null
   else { // retVec was null!
     
     if( sfGetError() == enaENOSUCHPRESET ) { 
        cout<<"Error, the MIDI Bank/MIDI Preset combination not found in bank"
            << endl; 
        return; 
     }
     else if( sfGetError() == enaENOSUCHBANK ) { 
	cout<<"Error, "<< sfBank <<" is an invalid bank ID."<<endl; 
        return; 
     }
   }
  return ; 

 }  // end doNav

void doGetPresets()
//*****************************************************************************
// 
// Implementation Notes: 
//
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
WORD            lineCount  = 0; 
SFPRESETHDRPTR  presetHdrs = NULL;
WORD            prstCnt; 
sfBankID        whatBank; 
enaErrors       omega;
CHAR            n; 

 cout<<"%getPresets-I- what enabler bankID? ";  cout.flush(); 
 cin >> whatBank; 
 cout<<endl; 

 presetHdrs = sfGetPresetHdrs( whatBank, &prstCnt); 

 if(presetHdrs == NULL) { 
   omega = sfGetError(); 
   if(omega == enaENOSUCHBANK ) { 
     cout<<"..."<<whatBank<<" is an invalid bank id. " << endl; 
     return; 
   }
   else if(omega == enaWBANKNOTLOADED) { 
    cout<<"..."<<whatBank<<" does not currently point at a loaded bank"<<endl;
    cout<<"List Loaded Banks for all valid IDs." <<endl; 
    return; 
   }
 }
 else {

  for(WORD curPreset=0; curPreset< prstCnt; curPreset++) { 

    cout<<"  Preset    >" << presetHdrs[curPreset].achPresetName<<"< "<< endl; 
    cout<<"    sfBank Index       : "<<curPreset << endl; 
    cout<<"    Midi Bank   Num    : "<<presetHdrs[curPreset].wPresetBank 
	<<endl;
    cout<<"    Midi Preset Num    : "<<presetHdrs[curPreset].wPresetNum 
	<<endl << endl; 
    lineCount += 5;

    if( lineCount != 0 ) {
       if( lineCount >= pageSize)  { // time for a page break/hold till return
         cout << endl << "---- more ----[n|q] " << endl; 
         cin >> n;
         lineCount = 0; 
         if(n=='q') break;  
       }
    }
  }
}
return;
}// end doGetPresets


void doGetSampleHdrs()
//*****************************************************************************
// 
// Implementation Notes: 
//    Query the user for the BankID of interest,  make the call to the enabler,
// check for errors, format the result  hdrs, and return. Be _sure_ not to 
// try and deallocate sampHdrs.
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
WORD            lineCount = 0; 
SFSAMPLEHDRPTR  sampHdrs; 
WORD            sampCnt; 
sfBankID        whatBank; 
enaErrors       omega;
CHAR            n; 

 cout<<"%getSampHdrs-I- what enabler bankID? ";  cout.flush(); 
 cin >> whatBank; 
 cout<<endl; 
 
 sampHdrs = sfGetSampHdrs( whatBank, &sampCnt); 

 if(sampHdrs == NULL) { 
   omega = sfGetError(); 
   if ( omega == enaENOSUCHBANK ) {
     cout<<"..."<< whatBank <<" is an invalid bank id "  << endl; 
     return; 
   }
   else if(omega == enaWBANKNOTLOADED) { 
    cout<<"..."<<whatBank<<" does not currently point at a loaded bank"<<endl;
    cout<<"List Loaded Banks for all valid IDs." <<endl; 
    return; 
   }
 }
 else {

  for(WORD curSamp=0; curSamp< sampCnt; curSamp++) { 

    cout<<"  Sample    >" << sampHdrs[curSamp].achSampleName<<"< "<< endl; 
    cout<<"    sfBank Index     : "<<curSamp << endl; 

    cout<<"    Start Addr       : " << sampHdrs[curSamp].dwStart     << endl;
    cout<<"    BeginLoop        : " << sampHdrs[curSamp].dwStartloop << endl;
    cout<<"    EndLoop          : " << sampHdrs[curSamp].dwEndloop   << endl;
    cout<<"    End   Addr       : " << sampHdrs[curSamp].dwEnd       << endl; 
    cout<<"    Samp Size(bytes) : " 
        << (sampHdrs[curSamp].dwEnd - sampHdrs[curSamp].dwStart) * 2 << endl;
    lineCount += 7; 
    if( lineCount != 0 ) {
       if( lineCount >= pageSize)  { // time for a page break/hold till return
         cout << endl << "---- more ----[n|q] " << endl; 
         cin >> n;
         lineCount = 0; 
         if(n=='q') break;  
       }
    }
   }
 }
}// end doGetSampleHdrs;

void doDumpBankNodesArray()
//*****************************************************************************
// 
// Implementation Notes: 
//
//   Just call the pesky little routine...
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
  sfDumpBanksInfo(); 
  return; 
}// end doDumpBankNddesArray; 


void doPrintMainMenu()
//*****************************************************************************
// 
// Implementation Notes: 
//
//   Print the main menu. Misnomer, there are no submenus... Yet...
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May 95       Initial Creation, unit driver - Enabler
//****************************************************************************
{
 cout <<"-------------Enabler Main Menu-------------------------------" 
      << endl;
 cout << endl;
 cout << " 0    :  Quit                " <<  endl; 
 cout << " 1    :  Menu                " <<  endl; 
 cout << " 2    :  Load Bank           " <<  endl; 
 cout << " 3    :  Unload Bank         " <<  endl;
 cout << " 4    :  Navigate Preset     " <<  endl; 
 cout << " 5    :  Print Preset Hdrs   " <<  endl; 
 cout << " 6    :  Print Sample Hdrs   " <<  endl; 
 cout << " 7    :  List Loaded Banks   " <<  endl; 
 cout << endl;
 cout << "-----------------------------------------------------------" << endl;
return;
}// end doPrintMainMenu;


main ( int argc, char *argv[] )
//-****************************************************************************
// 
// Implementation Notes: 
//
//   MainLine routine for the enaber driver/test thing.
// Set the gobal pageSize if present, controls how many output lines before
// a pagebreak in routines that do such things. 
// Other than that, just get the next command and drop into the switch
// statement, call the desired routine. 
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May '95      Initial Creation Enabler test
//-***************************************************************************
{
mainMenuCmdType curCmd = menu; 

 if(argc >= 2 ) { 
    pageSize = atoi(argv[1]); 
 }
 doPrintMainMenu(); 

 while ( curCmd != quit ) {

  curCmd = (mainMenuCmdType)getCmd(); 

  switch (curCmd) { 

   case quit: 
     break; 

   case menu: 
     doPrintMainMenu();
     break; 

   case load:
     doLoadBank(); 
     break; 

   case unload:
     doUnloadBank(); 
     break; 

   case  navigate:
      doNav(); 
      break; 

   case  getPresets:
      doGetPresets(); 
      break; 

   case getSampleHdrs: 
      doGetSampleHdrs(); 
      break;

   case dumpBankNodesArray:
      doDumpBankNodesArray();
      break; 

   default: 

     cout << "unknown command, try again..." << endl;  
     doPrintMainMenu();
     curCmd = menu;

   } // end switch
 }// end while not quit

 cout <<"%"<< argv[0] <<"-I-quiting.... " << endl; 

}// end main
