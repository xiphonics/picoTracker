
#include "Mixer.h"

Mixer::Mixer():Persistent("MIXER")  {
	Clear() ;
} ;

Mixer::~Mixer() {
} ;

void Mixer::Clear() {

	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		channelBus_[i]=i ;
	}
} ;

void Mixer::SaveContent(TiXmlNode *node) {
} ;

 void Mixer::RestoreContent(TiXmlElement *element) {
}
