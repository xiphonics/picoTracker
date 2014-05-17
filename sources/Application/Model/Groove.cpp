
#include "Groove.h"

unsigned char Groove::data_[MAX_GROOVES][16] ;

Groove::Groove():Persistent("GROOVES") {
	Clear() ;
} ;

Groove::~Groove() {
} ;

void Groove::Clear() {
	// Init all grooves with basic datas
	memset(data_,NO_GROOVE_DATA,MAX_GROOVES*0xF) ;
	for (int i=0;i<MAX_GROOVES;i++) {
		data_[i][0]=6 ;
		data_[i][1]=6 ;
	} ;
	// init grooves selectah
	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		// Channel
		channelGroove_[i].groove_=0 ;
		channelGroove_[i].position_=0 ;
		channelGroove_[i].ticks_=data_[0][0] ;
	} ;
} ;
// Resest groove data at song startup

void Groove::Reset() {
	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		//Channel
		ChannelGroove &c=channelGroove_[i] ;
		c.position_=0 ;
		c.ticks_=data_[c.groove_][c.position_] ;
	}
} ;

void Groove::GetChannelData(int channel,int *groove,int *position) {
		ChannelGroove &c=channelGroove_[channel] ;
		*groove=c.groove_ ;
		*position=c.position_ ;
} ;

void Groove::SaveContent(TiXmlNode *node) {
	 saveHexBuffer(node,"DATA",(unsigned char *)data_,16*MAX_GROOVES) ;
} ;

 void Groove::RestoreContent(TiXmlElement *element) {
 	TiXmlElement *current=element->FirstChildElement() ;
	restoreHexBuffer(current,(unsigned char*)data_) ;
}
// Trigger grooves so we go to the next step

void Groove::Trigger() {
	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		ChannelGroove &c=channelGroove_[i] ;
		UpdateGroove(c,false) ;
	}
} ;

bool Groove::UpdateGroove(ChannelGroove &c,bool reverse) {

	bool stepped=false ;

	if (reverse) { //Table
		c.ticks_++ ;
		if (c.groove_==255) { // Default table groove
			if (c.ticks_==1) {
				stepped=true  ;
				c.ticks_=0 ;
			}
		} else {
			if (c.ticks_==data_[c.groove_][c.position_]) {
				c.position_=(c.position_+1)%16 ;
				if (data_[c.groove_][c.position_]==0xFF) {
					c.position_=0 ;
				} ;
				c.ticks_=0 ;
				stepped=true ;
			}
		}
	} else {  // Note
		if (c.ticks_==0) {
			c.position_=(c.position_+1)%16 ;
			if (data_[c.groove_][c.position_]==0xFF) {
				c.position_=0 ;
			} ;
			c.ticks_=data_[c.groove_][c.position_] ;
			stepped=true ;
		} ;
		c.ticks_-- ;
	}
	return stepped ;
}

void Groove::SetGroove(int channel,int groove) {
		if (groove>=MAX_GROOVES) return ;
		channelGroove_[channel].groove_=groove ;
		channelGroove_[channel].position_=0 ;
		channelGroove_[channel].ticks_=data_[channelGroove_[channel].groove_][channelGroove_[channel].position_] ;
} ;

// Returns true if, according to current groove setting it is time to go
// to the next sequencing step

bool Groove::TriggerChannel(int i) {
	ChannelGroove &c=channelGroove_[i] ;
	return ((c.ticks_)%(data_[c.groove_][c.position_])==0) ;
} ;

unsigned char *Groove::GetGrooveData(int groove) {
	return data_[groove] ;
} ;
