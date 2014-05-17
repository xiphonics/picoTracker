#include "MidiInstrument.h"
#include "CommandList.h"
#include "System/Console/Trace.h"
#include <string.h>

MidiService *MidiInstrument::svc_=0 ;

MidiInstrument::MidiInstrument() {

	strcpy(name_,"0") ;
	
	if (svc_==0) {
		svc_=MidiService::GetInstance() ;
	};

	Variable *v=new Variable("channel",MIP_CHANNEL,0) ;
	Insert(v) ;
	v=new Variable("note length",MIP_NOTELENGTH,0) ;
	Insert(v) ;
	v=new Variable("volume",MIP_VOLUME,255) ;
	Insert(v) ;
	v=new Variable("table",MIP_TABLE,-1) ;
	Insert(v) ;
	v=new Variable("table automation",MIP_TABLEAUTO,false) ;
	Insert(v) ;
}

MidiInstrument::~MidiInstrument() {
} ;

bool MidiInstrument::Init() {
	tableState_.Reset() ;
	return true ;
};

void MidiInstrument::OnStart() {
	tableState_.Reset() ;
} ;

bool MidiInstrument::Start(int c,unsigned char note,bool retrigger) {

	first_[c]=true ;
	lastNote_[c]=note ;

	Variable *v=FindVariable(MIP_CHANNEL) ;
	int channel=v->GetInt() ;

	v=FindVariable(MIP_NOTELENGTH) ;
	remainingTicks_=v->GetInt() ;
	if (remainingTicks_==0) {
        remainingTicks_=-1 ;
    }

	MidiMessage msg ;

	//	send volume

	v=FindVariable(MIP_VOLUME) ;
	msg.status_=MIDI_CC+channel ;
	msg.data1_=7 ;
	msg.data2_=int((v->GetInt()+0.99)/2) ;
	svc_->QueueMessage(msg) ;


    playing_=true ;
	retrig_=false ;

	return true ;
} ;

void MidiInstrument::Stop(int c) {

	Variable *v=FindVariable(MIP_CHANNEL) ;
	int channel=v->GetInt() ;

	MidiMessage msg;
	msg.status_=MIDI_NOTE_OFF+channel ;
	msg.data1_=lastNote_[c] ;
	msg.data2_=0x00 ;
	svc_->QueueMessage(msg) ;
	playing_=false ;

} ;

void MidiInstrument::SetChannel(int channel) {
	Variable *v=FindVariable(MIP_CHANNEL) ;
	v->SetInt(channel) ;
} ;

bool MidiInstrument::Render(int channel,fixed *buffer,int size,bool updateTick) {

	// We do it here so we have the opportunity to send some command before

	Variable *v=FindVariable(MIP_CHANNEL) ;
	int mchannel=v->GetInt() ;
	if (first_[channel]) {

		// send note

		MidiMessage msg ;

		msg.status_=MIDI_NOTE_ON+mchannel ;
		msg.data1_=lastNote_[channel] ;
		msg.data2_=0x7F ;
		svc_->QueueMessage(msg) ;

		first_[channel]=false ;
	}
	if (remainingTicks_>0) {
        remainingTicks_-- ;
        if (remainingTicks_==0) {
			if (!retrig_) {
	            Stop(channel) ;
			} else {
				MidiMessage msg ;
				remainingTicks_=retrigLoop_ ;
				msg.status_=MIDI_NOTE_OFF+mchannel ;
				msg.data1_=lastNote_[channel] ;
				msg.data2_=0x00 ;
				svc_->QueueMessage(msg) ;
				msg.status_=MIDI_NOTE_ON+mchannel ;
				msg.data1_=lastNote_[channel] ;
				msg.data2_=0x7F ;
				svc_->QueueMessage(msg) ;
			} ;
        } ;
    } ;
	return false ; 
};

bool MidiInstrument::IsInitialized() {
	return true ; // Always initialised
} ;

void MidiInstrument::ProcessCommand(int channel,FourCC cc,ushort value) {

	Variable *v=FindVariable(MIP_CHANNEL) ;
	int mchannel=v->GetInt() ;

	switch(cc) {

		case I_CMD_RTRG:
            {
				unsigned char loop=(value&0xFF) ; // number of ticks before repeat
                if (loop!=0) {
                    retrig_=true ;
                    retrigLoop_=loop ;
                    remainingTicks_=loop ;
                } else {
                    retrig_=false ;
                }
            }
			break ;

		case I_CMD_VOLM:
			{
				MidiMessage msg ;
				msg.status_=MIDI_CC+mchannel ;
				msg.data1_=7 ;
				msg.data2_=int((value+0.99)/2) ;
				svc_->QueueMessage(msg) ;
			} ;
			break ;

		case I_CMD_MDCC:
			{
				MidiMessage msg ;
				msg.status_=MIDI_CC+mchannel ;
				msg.data1_=(value&0x7F00)>>8 ;
				msg.data2_=(value&0x7F) ;
				svc_->QueueMessage(msg) ;
			};
			break ;

		case I_CMD_MDPG:
			{
				MidiMessage msg ;
				msg.status_=MIDI_PRG+mchannel ;
				msg.data1_=(value&0x7F) ;
				msg.data2_=MidiMessage::UNUSED_BYTE ;
				svc_->QueueMessage(msg) ;
			};
			break ;
	}
} ;

const char *MidiInstrument::GetName() {
	Variable *v=FindVariable(MIP_CHANNEL) ;
	sprintf(name_,"MIDI CH %2.2d",v->GetInt()+1) ;
    return name_ ;
}

int MidiInstrument::GetTable() {
	Variable *v=FindVariable(MIP_TABLE) ;
	return v->GetInt() ;
} ;

bool MidiInstrument::GetTableAutomation() {
	Variable *v=FindVariable(MIP_TABLEAUTO) ;
	return v->GetBool() ;
} ;

void MidiInstrument::GetTableState(TableSaveState &state) {
	memcpy(state.hopCount_,tableState_.hopCount_,sizeof(uchar)*TABLE_STEPS*3) ;
	memcpy(state.position_,tableState_.position_,sizeof(int)*3) ;
} ;

void MidiInstrument::SetTableState(TableSaveState &state) {
	memcpy(tableState_.hopCount_,state.hopCount_,sizeof(uchar)*TABLE_STEPS*3) ;
	memcpy(tableState_.position_,state.position_,sizeof(int)*3) ;
} ;
