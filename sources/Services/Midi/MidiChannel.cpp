
#include "MidiChannel.h"

#define CONTROLLER_SPEED_UPDATE_FACTOR 0.5f

MidiChannel::MidiChannel(const char *name):Channel(name) {
	controllerType_=MCT_NONE ;
	toggle_=false ;
	lastVal_=-1 ;
	range_=256 ;
	circular_=false ;
} ;

MidiChannel::~MidiChannel() {
} ;

void MidiChannel::SetControllerType(MidiControllerType type) {
	controllerType_=type ;
} ;

MidiControllerType MidiChannel::GetControllerType() {
	return controllerType_ ;
};

void MidiChannel::SetToggle(bool toggle) {
	toggle_=toggle ;
} ;

bool MidiChannel::IsToggle() {
	return toggle_ ;
};

void MidiChannel::SetCircular(bool circular) {
	circular_=circular ;
} ;

bool MidiChannel::IsCircular() {
	return circular_ ;
};

void MidiChannel::SetHiRes(bool hires) {
	hiRes_=hires ;
} ;

bool MidiChannel::IsHiRes() {
	return hiRes_ ;
};
int MidiChannel::GetRange() {
	return range_ ;
}

void MidiChannel::SetRange(int range) {
	range_=range ;
} ;