#ifndef _PROJECT_H_
#define _PROJECT_H_

#include "Song.h"
#include "Application/Instruments/InstrumentBank.h"
#include "Application/Persistency/Persistent.h"
#include "Foundation/Variables/VariableContainer.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Observable.h"


#define VAR_TEMPO       MAKE_FOURCC('T','M','P','O')
#define VAR_MASTERVOL   MAKE_FOURCC('M','S','T','R')
#define VAR_WRAP        MAKE_FOURCC('W','R','A','P')
#define VAR_MIDIDEVICE  MAKE_FOURCC('M','I','D','I')
#define VAR_TRANSPOSE  MAKE_FOURCC('T','R','S','P')

#define PROJECT_NUMBER "1.0"
#define PROJECT_RELEASE "r"
#define BUILD_COUNT "003"

#define MAX_TAP 3

class Project: public Persistent,public VariableContainer,I_Observer  {
public:
	Project() ;
	~Project() ;
	void Purge() ;
	void PurgeInstruments(bool removeFromDisk) ;

	Song *song_ ;
 
	int GetMasterVolume() ;
	bool Wrap() ;
	void OnTempoTap();
	void NudgeTempo(int value) ;
	int GetTempo() ; // Takes nudging into account
	int GetTranspose() ;

	void Trigger() ;

	// I_Observer
    virtual void Update(Observable &o,I_ObservableData *d);
 
	InstrumentBank* GetInstrumentBank() ;
  virtual void SaveContent(tinyxml2::XMLPrinter *printer);
  virtual void RestoreContent(PersistencyDocument *doc);

protected:
	void buildMidiDeviceList() ;
private:
	InstrumentBank *instrumentBank_ ;
	char **midiDeviceList_ ;
	int midiDeviceListSize_ ;
	int tempoNudge_ ;
	unsigned long lastTap_[MAX_TAP] ;
	unsigned int tempoTapCount_ ; 
} ;

#endif

