#ifndef _NDS_SOUND_H_
#define _NDS_SOUND_H_

#include "System/Sound/Sound.h"
#include "Adapters/NDS/Command/Command.h"

#define SOUND_BUFFER_COUNT 20
#define SOUND_BUFFER_MAX 20000


class NDSSound: public Sound {
public:
     // Sound implementation
	virtual bool Init() ; 
	virtual void Close();
	virtual bool Start() ; 
	virtual void Stop();
	virtual void QueueBuffer(char *buffer,int len) ;
	virtual int GetPlayedBufferPercentage() ;
    virtual int GetSampleRate() ;
    void onChunkDone() ;
    virtual bool Interlaced() { return true ; } ;
protected:
    void startARM7() ;
    void stopARM7() ;
private:

    unsigned short *ringBufferL_ ;    	
    unsigned short *ringBufferR_ ;  
    
    unsigned short *ringBoundaries_[2][RING_BUFFER_COUNT] ;
    int currentBoundary_ ;
    
    bool isPlaying_ ;
    char *pool_[SOUND_BUFFER_COUNT] ;
    int poolSize_[SOUND_BUFFER_COUNT] ;
    int poolQueuePosition_ ;
    int poolPlayPosition_ ;
	char *unalignedMain_ ;
    char *mainBuffer_ ;
    char *miniBlank_ ;
    int bufferPos_ ;
    int bufferSize_ ;
	int volume_ ;      	
	int lastTimer_ ;
} ;
#endif
