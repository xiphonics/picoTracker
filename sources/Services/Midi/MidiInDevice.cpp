#include "MidiInDevice.h"
#include "System/System/System.h"
#include "System/Console/Trace.h"
#include "Application/Model/Config.h"

using namespace std;

bool MidiInDevice::dumpEvents_ = false;

MidiInDevice::MidiInDevice(const char *name):ControllerSource("midi",name),T_Stack<MidiMessage>(true) {

  const char *dumpIt=Config::GetInstance()->GetValue("DUMPEVENT") ;
  dumpEvents_ = (dumpIt!=0);

	for (int channel = 0; channel < 16; channel++) {
		for (int i=0;i<128;i++) {
			noteChannel_[channel][i] = NULL;
			ccChannel_[channel][i] = NULL;
			atChannel_[channel][i] = NULL;
		}
		pbChannel_[channel] = NULL;
		catChannel_[channel] = NULL;
    pcChannel_[channel] = NULL;
	}

	isRunning_=false ;
} ;

MidiInDevice::~MidiInDevice() {
	
	for  (int channel = 0; channel < 16; channel++) {
		for (int i=0;i<128;i++) {
			SAFE_DELETE(noteChannel_[channel][i]);
			SAFE_DELETE(ccChannel_[channel][i]);
			SAFE_DELETE(atChannel_[channel][i]);
		}
		SAFE_DELETE(pbChannel_[channel]);
		SAFE_DELETE(catChannel_[channel]);
		SAFE_DELETE(pcChannel_[channel]);
	}
} ;

bool MidiInDevice::Init() {
	return initDriver() ;
} ;

void MidiInDevice::Close() {
	closeDriver() ;
} ;

bool MidiInDevice::Start() {
	isRunning_=true ;
	return startDriver() ;
} ;

void MidiInDevice::Stop() {
	isRunning_=false ;
	stopDriver() ;
} ;

bool MidiInDevice::IsRunning() {
	return isRunning_ ;
} ;
/*
void MidiInDevice::onMidiStart() {
	MidiSyncData data(MSM_START) ;
	SetChanged() ;
	NotifyObservers() ;
} ;

void MidiInDevice::onMidiStop() {
	MidiSyncData data(MSM_STOP) ;
	SetChanged() ;
	NotifyObservers() ;
} ;

void MidiInDevice::onMidiTempoTick() {
	MidiSyncData data(MSM_TEMPOTICK) ;
	SetChanged() ;
	NotifyObservers() ;
} ;

void MidiInDevice::queueEvent(MidiEvent &event) {
	T_Stack<MidiEvent>::Insert(event) ;
} ;
*/

void MidiInDevice::onDriverMessage(MidiMessage &message) {
	SetChanged() ;
	NotifyObservers(&message) ;
	treatChannelEvent(message) ;
} ;

void MidiInDevice::Trigger(Time time) {

	MidiMessage *event=Pop(true) ;
	while (event) {
		treatChannelEvent(*event) ;
		delete event ;

		event=Pop(true) ;
	} ;



	for (int midiChannel = 0; midiChannel < 16; midiChannel++) {
		for (int i=0;i<128;i++) {
			if (ccChannel_[midiChannel][i]) {
				ccChannel_[midiChannel][i]->Trigger() ;
			} ;
			if (noteChannel_[midiChannel][i]) {
				noteChannel_[midiChannel][i]->Trigger() ;
			}
			if (atChannel_[midiChannel][i]) {
				atChannel_[midiChannel][i]->Trigger() ;
			}
		} ;
		if (pbChannel_[midiChannel]) {
			pbChannel_[midiChannel]->Trigger() ;
		} ;
		if (pcChannel_[midiChannel]) {
			pcChannel_[midiChannel]->Trigger() ;
		}
		if (catChannel_[midiChannel]) {
			catChannel_[midiChannel]->Trigger() ;
		} ;
	}
} ;

void MidiInDevice::treatChannelEvent(MidiMessage &event) {

	int midiChannel = event.status_ & 0x0F;

	bool isMidiClockEvent = (event.status_ == 0xF8);

	switch (event.GetType())
  {
    case MidiMessage::MIDI_NOTE_OFF:
		{
			int note=event.data1_&0x7F ;

			if (noteChannel_[midiChannel][note]!=0)
      {
				treatNoteOff(noteChannel_[midiChannel][note]) ;
			}
		}
		break;
      
		case MidiMessage::MIDI_NOTE_ON:
		{
			int note=event.data1_&0x7F ;
			int data=event.data2_&0x7F ;
      if (dumpEvents_)
      {
        Trace::Log("EVENT","midi:note:%d",note) ;
      }
			if (noteChannel_[midiChannel][note]!=0)
      {
				treatNoteOn(noteChannel_[midiChannel][note],data) ;
			}
		}
		break;
      
		case MidiMessage::MIDI_AFTERTOUCH:
		{
			int note=event.data1_&0x7F ;
			int data=event.data2_&0x7F ;
      
      MidiChannel* channel = atChannel_[midiChannel][note];
			if (channel)
      {
				channel->SetValue(float(data)/(channel->GetRange()/2.0f-1)) ;
        channel->Trigger();
			}
		}
		break ;
      
      
		case MidiMessage::MIDI_CONTROLLER:
		{
			int cc=event.data1_&0x7F ;
			int data=event.data2_&0x7F ;
      
      if (dumpEvents_)
      {
        Trace::Log("EVENT","midi:cc:%d:%d",cc,data) ;
      }

			// First, look if we're not handling a hi nibble from a hi-res CC
			
			if ((cc>31)&&(ccChannel_[midiChannel][cc-32]!=0)&&(ccChannel_[midiChannel][cc-32]->IsHiRes())) {
				treatCC(ccChannel_[midiChannel][cc-32],data) ;
			} else {
				if (ccChannel_[midiChannel][cc]!=0) {
					if (ccChannel_[midiChannel][cc]->IsHiRes()) {
						treatCC(ccChannel_[midiChannel][cc],data,true) ;
					} else {
						treatCC(ccChannel_[midiChannel][cc],data) ;
					}
				} ;
			}
		}
      break;
      
		case MidiMessage::MIDI_PROGRAM_CHANGE:
		{
			int data=event.data1_&0x7F ;
      
      MidiChannel* channel = pcChannel_[midiChannel];
			if (channel)
      {
				channel->SetValue(float(data)/(channel->GetRange()/2.0f-1)) ;
        channel->Trigger();
			}
		}
      break ;
		
    case MidiMessage::MIDI_CHANNEL_AFTERTOUCH:
		{
			int data=event.data1_&0x7F ;

      MidiChannel* channel = catChannel_[midiChannel];
			if (channel)
      {
				channel->SetValue(float(data)/(channel->GetRange()/2.0f-1)) ;
        channel->Trigger();
			}
		}
		break ;

		case MidiMessage::MIDI_PITCH_BEND:
		{
      MidiChannel* channel = pbChannel_[midiChannel];
			if (channel)
      {
				channel->SetValue((event.data2_*0x7F+event.data1_)/float(0x3F80)) ;
        channel->Trigger();
			} ;
		}
		case 0xF0: // Midi clock
			break ;
		default:
			break;
	} ;
} ;

Channel *MidiInDevice::GetChannel(const char *sourcePath) {

	Channel *channel=0 ;

	string path=sourcePath ;
	string::size_type pos = path.find (":",0);
	if (pos == string::npos) {
		return 0 ;
	} ;


	MidiChannel **ccChannel=0 ;
	MidiChannel **noteChannel=0 ;
	MidiChannel **pbChannel=0 ;
	MidiChannel **pcChannel=0 ;
	MidiChannel **catChannel=0 ;
	MidiChannel **atChannel=0 ;
	MidiChannel **activityChannel=0 ;

	string firstElem=path.substr (0,pos);
	string type ="" ;



	// MIDI channel dependent channels

	string &midiChannelStr = firstElem;

	// First read the channel number

	int midiChannel=atoi(midiChannelStr.c_str()) ;
	if ((midiChannel<0)||(midiChannel>15)) {
		return 0 ;
	} ;
	 ;

	// read type
	path = path.substr(pos+1) ;
	pos = path.find (":",0);
	type = path.substr (0,pos);

	// Read the event id (note, cc, pb)
	pos = path.find (":",0);
	path = path.substr(pos+1) ;
	int id=atoi(path.c_str()) ;
	if ((id<0)||(id>127)) {
		return 0 ;
	} ;

	ccChannel=&(ccChannel_[midiChannel][id]);
	noteChannel=&(noteChannel_[midiChannel][id]);
	pbChannel=&(pbChannel_[midiChannel]);
	catChannel=&(catChannel_[midiChannel]);
	atChannel=&(atChannel_[midiChannel][id]);
	pcChannel=&(pcChannel_[midiChannel]);


	if (type.substr(0,2)=="cc") {

		MidiControllerType ccType=MCT_NONE ;
		bool isCircular=false ;
		bool isHiRes=false ;

		if (type[2]!=0) {
			switch(type[2]) {
				case L'+':
					ccType=MCT_2_COMP ;
					break ;
				case L'|':
					ccType=MCT_HIRES;
					isHiRes=true ;
					break ;
				case L'-':
					ccType=MCT_SIGNED_BIT;
					break ;
				case L'_':
					ccType=MCT_SIGNED_BIT_2;
					break ;
				case L'=':
					ccType=MCT_BIN_OFFSET;
					break ;
				case L'*': // backward compatibility
					ccType=MCT_2_COMP ;
					isCircular=true ;
					//assert(0) ;
					break ;
			}
			if (type[3]!=0) {
				NAssert(type[3]==L'*') ;
				isCircular=true ;
			} ;
		} ;


		if (*ccChannel==0) {
			*ccChannel=new MidiChannel(sourcePath) ;
		}
		(*ccChannel)->SetControllerType(ccType) ;
		(*ccChannel)->SetCircular(isCircular) ;
		(*ccChannel)->SetHiRes(isHiRes) ;
		channel=*ccChannel ;
	} ;

	if (type=="note") {
		if (*noteChannel==0) {
			*noteChannel=new MidiChannel(sourcePath) ;
		};
		channel=*noteChannel ;
	} ;
	if (type=="note+") {
		if (*noteChannel==0) {
			*noteChannel=new MidiChannel(sourcePath) ;
			(*noteChannel)->SetToggle(true) ;
		};
		channel=*noteChannel ;
	} ;
	if (type=="at") {
		if (*atChannel==0) {
			(*atChannel)=new MidiChannel(sourcePath) ;
		};
		channel=*atChannel ;
	} ;
	if (type=="pb") {
		if (*pbChannel==0) {
			*pbChannel=new MidiChannel(sourcePath) ;
		};
		channel=*pbChannel ;
	} ;
	if (type=="cat") {
		if (*catChannel==0) {
			*catChannel=new MidiChannel(sourcePath) ;
		};
		channel=*catChannel ;
	} ;
	if (type=="pc") {
		if (*catChannel==0) {
			*catChannel=new MidiChannel(sourcePath) ;
		};
		channel=*pcChannel ;
	} ;
	if (type=="activity") {
		if (*activityChannel==0) {
			*activityChannel=new MidiChannel(sourcePath) ;
		};
		channel=*activityChannel ;
	} ;
	return channel ; ;
} ;

void MidiInDevice::treatCC(MidiChannel *channel,int data,bool hiNibble) {
	switch(channel->GetControllerType()) {
		// Regular midi channels
		case MCT_NONE: // to cope with the fact we want to have the possibility to map
					   // MIDI controllers to 0.5,0.25, etc, we map differently if data
					   // is zero or otherwise.

			channel->SetValue((data>0)?float(data+1)/(channel->GetRange()/2.0f):0) ;
			break ;
		case MCT_HIRES:
			{
			float channelValue=channel->GetValue() ;
			int current=(channelValue==0)?0:int(channelValue*16384-1) ;
			int hi=current/128 ;
			int low=current-hi*128 ;
			if (hiNibble) {
				hi=data ;
			} else {
				low=data ;
			}
			current=hi*128+low ;
			channel->SetValue((current==0)?0:(current+1)/16384.0f) ;
			}
			break ;
		case MCT_2_COMP:
		{
			float current=channel->GetValue() ;
			int incr=0 ;
			if (data!=0)  {
				if (data<0x41) {
					incr=data ;
				} else {
					incr=data-0x80 ;
				} ;
			} ;
			current+=float(incr)/(channel->GetRange()/2.0f-1.0f) ;
			if (channel->IsCircular()) {
				if (current>1.0) current-=1.0F ;
				if (current<0.0) current+=1.0F ;						
			} else {
				if (current>1.0) current=1.0F ;
				if (current<0.0) current=0.0F ;
			}
			channel->SetValue(current) ;
			break ;
		}
		case MCT_SIGNED_BIT:
		{
			float current=channel->GetValue() ;
			int incr=0 ;
			if (data!=0)  {
				if (data<0x41) {
					incr=data ;
				} else {
					incr=0x40-data ;
				} ;
			} ;
			current+=float(incr)/(channel->GetRange()/2.0f-1.0f) ;
			if (channel->IsCircular()) {
				if (current>1.0) current-=1.0F ;
				if (current<0.0) current+=1.0F ;						
			} else {
				if (current>1.0) current=1.0F ;
				if (current<0.0) current=0.0F ;
			}
			channel->SetValue(current) ;
			break ;
		}
		case MCT_SIGNED_BIT_2:
		{
			float current=channel->GetValue() ;
			int incr=0 ;
			if (data!=0)  {
				if (data<0x41) {
					incr=-data ;
				} else {
					incr=data-0x40 ;
				} ;
			} ;
			current+=float(incr)/(channel->GetRange()/2.0f-1.0f) ;
			if (channel->IsCircular()) {
				if (current>1.0) current-=1.0F ;
				if (current<0.0) current+=1.0F ;						
			} else {
				if (current>1.0) current=1.0F ;
				if (current<0.0) current=0.0F ;
			}
			channel->SetValue(current) ;
			break ;
		}
		case MCT_BIN_OFFSET:
		{
			float current=channel->GetValue() ;
			int incr=0 ;
			if (data!=0)  {
				if (data<0x41) {
					incr=data-0x40 ;
				} else {
					incr=data-0x40 ;
				} ;
			} ;
			current+=float(incr)/(channel->GetRange()/2.0f-1.0f) ;
			if (channel->IsCircular()) {
				if (current>1.0) current-=1.0F ;
				if (current<0.0) current+=1.0F ;						
			} else {
				if (current>1.0) current=1.0F ;
				if (current<0.0) current=0.0F ;
			}
			channel->SetValue(current) ;
			break ;
		}
	} ;
	channel->Trigger() ;
} ;

void MidiInDevice::treatNoteOff(MidiChannel *channel) {
	if (!channel->IsToggle()) {
		channel->SetValue(0.0F) ;
	}
	channel->Trigger() ;
} ;

void MidiInDevice::treatNoteOn(MidiChannel *channel,int data) {
	if (data==0) { // Actually a note off
		if (!channel->IsToggle()) {
			channel->SetValue(0.0F) ;
		}
	} else {
		if (!channel->IsToggle()) {
			channel->SetValue(1.0F) ;
		} else {
			float current=channel->GetValue() ;
			channel->SetValue((current>0.5)?0.0f:1.0f) ;
		} ;
	}
	channel->Trigger() ;
} ;
