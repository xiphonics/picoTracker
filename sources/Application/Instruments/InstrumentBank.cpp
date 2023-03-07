
#include "InstrumentBank.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Instruments/MidiInstrument.h"
#include "System/io/Status.h"
#include "Application/Utils/char.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Filters.h"

char *InstrumentTypeData[IT_LAST]= {
	"Sample",
	"Midi"
} ;


// Contain all instrument definition

InstrumentBank::InstrumentBank():Persistent("INSTRUMENTBANK") {

   	for (int i=0;i<MAX_SAMPLEINSTRUMENT_COUNT;i++) {
      Trace::Debug("Loading sample instrument: %i", i);
      SampleInstrument *s = new SampleInstrument();
      instrument_[i] = s;
    }
	for (int i=0;i<MAX_MIDIINSTRUMENT_COUNT;i++) {
    Trace::Debug("Loading MIDI instrument: %i", i);
    MidiInstrument *s = new MidiInstrument();
    s->SetChannel(i);
    instrument_[MAX_SAMPLEINSTRUMENT_COUNT + i] = s;
  }
  Status::Set("All instrument loaded") ;
} ;

//
// Assigns default instruments value for new project
//

void InstrumentBank::AssignDefaults() {

	SamplePool *pool=SamplePool::GetInstance() ;
   	for (int i=0;i<MAX_SAMPLEINSTRUMENT_COUNT;i++) {
		SampleInstrument *s=(SampleInstrument*)instrument_[i] ;
		if (i<pool->GetNameListSize()) {
	        s->AssignSample(i) ;
		} else {
			s->AssignSample(-1) ;
		} 
    } ;
} ;

InstrumentBank::~InstrumentBank() {
	for (int i=0;i<MAX_INSTRUMENT_COUNT;i++) {
		delete instrument_[i] ;
	}	
} ;

I_Instrument *InstrumentBank::GetInstrument(int i) {
	return instrument_[i] ;
} ;

void InstrumentBank::SaveContent(TiXmlNode *node) {
	char hex[3] ;
	for (int i=0;i<MAX_INSTRUMENT_COUNT;i++) {

		I_Instrument *instr=instrument_[i] ;
		if (!instr->IsEmpty()) {
			TiXmlElement data("INSTRUMENT") ;
			hex2char(i,hex) ;
			data.SetAttribute("ID",hex) ;
			data.SetAttribute("TYPE",InstrumentTypeData[instr->GetType()]) ;

			IteratorPtr<Variable> it(instr->GetIterator()) ;
			int count=0 ;
			for (it->Begin();!it->IsDone();it->Next()) {
				Variable &v=it->CurrentItem() ;
				TiXmlElement param("PARAM") ;
				param.SetAttribute("NAME",v.GetName()) ;
				param.SetAttribute("VALUE",v.GetString()) ;
				data.InsertEndChild(param) ;
				count++ ;
			}
			if (count) node->InsertEndChild(data) ;
		}
	}
} ;

void InstrumentBank::RestoreContent(PersistencyDocument *doc) {

  if (doc->version_ < 130)
  {
    if (Config::GetInstance()->GetValue("LEGACYDOWNSAMPLING") != NULL)
    {
      SampleInstrument::EnableDownsamplingLegacy();
    }
  }
  bool elem = doc->FirstChild();
  while (elem) {
    // Check it is an instrument
    if (!strcmp(doc->ElemName(), "INSTRUMENT")) {
      // Get the instrument ID
      unsigned char id;
      char *hexid = NULL;
      char *instype = NULL;
      bool hasAttr = doc->NextAttribute();
      while (hasAttr) {
        if (!strcmp(doc->attrname_, "ID")) {
          hexid = doc->attrval_;
          unsigned char b1 = (c2h__(hexid[0])) << 4;
          unsigned char b2 = c2h__(hexid[1]);
          id = b1 + b2;    
        }
        if (!strcmp(doc->attrname_, "TYPE")) {
          instype = doc->attrval_;
        }
        hasAttr = doc->NextAttribute();
      }

      InstrumentType it = IT_LAST;
      if (instype) {
        for (int i = 0; i < IT_LAST; i++) {
          if (!strcmp(instype, InstrumentTypeData[i])) {
            it = (InstrumentType)i;
            break;
          }
        }
      }
      else {
        it = (id < MAX_SAMPLEINSTRUMENT_COUNT) ? IT_SAMPLE : IT_MIDI;
      };
      if (id < MAX_INSTRUMENT_COUNT) {
        I_Instrument *instr=instrument_[id] ;
				if (instr->GetType()!=it) {
					delete instr ;
					switch (it) {
						case IT_SAMPLE:
							instr=new SampleInstrument() ;
							break ;
						case IT_MIDI:
							instr=new MidiInstrument() ;
							break ;
					}
					instrument_[id]=instr ;
				} ;

        bool subelem = doc->FirstChild();
        while (subelem) {
          bool hasAttr = doc->NextAttribute();
          char name[24];
          char value[24];
          while (hasAttr) {
            if (!strcmp(doc->attrname_, "NAME")) {
              strcpy(name, doc->attrval_);
            }
            if (!strcmp(doc->attrname_, "VALUE")) {
              strcpy(value, doc->attrval_);
            }
            hasAttr = doc->NextAttribute();
          }

          // Convert old filter dist to newer filter mode
          if (!strcmp(name, "filter dist")) {
            strcpy(name, "filter mode");
            if (!strcmp(value, "none")) {
              strcpy(value, "original");
            } else {
              strcpy(value, "scream");
            }
          }
          
          IteratorPtr<Variable> it(instr->GetIterator());
          for (it->Begin(); !it->IsDone(); it->Next()) {
            Variable &v = it->CurrentItem();
            if (!strcmp(v.GetName(), name)) {
              v.SetString(value);
            };
          }
          subelem = doc->NextSibling();
        }
        if (doc->version_<38) {
					Variable *cvl=instr->FindVariable(SIP_CRUSHVOL) ;
					Variable *vol=instr->FindVariable(SIP_VOLUME);
					Variable *crs=instr->FindVariable(SIP_CRUSH) ;
					if ((vol)&&(cvl)&&(crs)) {
						if (crs->GetInt()!=16) {
							int temp=vol->GetInt() ;
							vol->SetInt(cvl->GetInt()) ;
							cvl->SetInt(temp) ;
						}
					};
        }
			}
    }
    elem = doc->NextSibling();
  };
};

void InstrumentBank::Init() {
	for (int i=0;i<MAX_INSTRUMENT_COUNT;i++) {
		instrument_[i]->Init() ;
	}
}

unsigned short InstrumentBank::GetNext() {
	for (int i=0;i<MAX_SAMPLEINSTRUMENT_COUNT;i++) {
		SampleInstrument *si=(SampleInstrument *)instrument_[i] ;
		Variable *sample=si->FindVariable(SIP_SAMPLE) ;
		if (sample) {
			if (sample->GetInt()==-1) {
				return i ;
			}
		}
	}
	return NO_MORE_INSTRUMENT ;
} ;

unsigned short InstrumentBank::Clone(unsigned short i) {
	// can't clone midi instruments

	unsigned short next=GetNext() ;
	if (next==NO_MORE_INSTRUMENT) 
  {
		return NO_MORE_INSTRUMENT ;
	}

	I_Instrument *src=instrument_[i] ;
	I_Instrument *dst=instrument_[next] ;

  if (src == dst)
  {
		return NO_MORE_INSTRUMENT ;
	}

	delete dst ;
  
	if (src->GetType()==IT_SAMPLE) {
		dst=new SampleInstrument() ;
	} else {
		dst=new MidiInstrument() ;
	}
	instrument_[next]=dst ;
	IteratorPtr<Variable> it(src->GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Variable &srcV=it->CurrentItem() ;
		Variable *dstV=dst->FindVariable(srcV.GetID()) ;
		if (dstV) {
			dstV->CopyFrom(srcV) ;
		}
	}
	return next ;

}

void InstrumentBank::OnStart() {
	for (int i=0;i<MAX_INSTRUMENT_COUNT;i++) {
		instrument_[i]->OnStart() ;
	}
	init_filters() ;
} ;
