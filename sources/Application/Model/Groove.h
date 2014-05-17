
#ifndef _GROOVE_H_
#define _GROOVE_H_

#include "Foundation/T_Singleton.h"
#include "Application/Persistency/Persistent.h"
#include "Application/Model/Song.h"
#include "Application/Utils/HexBuffers.h"

#define MAX_GROOVES 0x20
#define NO_GROOVE_DATA 0xFF

struct ChannelGroove {
	unsigned char groove_ ; // selected groove
	unsigned char position_ ; // step in the groove table
	unsigned char ticks_ ; // number of ticks before next step

} ;

class Groove:public T_Singleton<Groove>,Persistent {
public:
	Groove() ;
	~Groove() ;
	void Reset() ;
	void Clear() ;
	void Trigger() ;
	bool TriggerChannel(int channel) ;
	void SetGroove(int channel,int groove) ;
	bool UpdateGroove(ChannelGroove &g,bool reverse) ;
	void GetChannelData(int channel,int *groove,int *position) ;
	unsigned char *GetGrooveData(int groove) ;
	virtual void SaveContent(TiXmlNode *node) ;
	virtual void RestoreContent(TiXmlElement *element);
	
private:
	ChannelGroove channelGroove_[SONG_CHANNEL_COUNT] ; 
	static unsigned char data_[MAX_GROOVES][16] ;
} ;
#endif

