#ifndef _AUDIO_OUT_DRIVER_H_
#define _AUDIO_OUT_DRIVER_H_

#include "AudioOut.h"
#include "Foundation/Observable.h"
#include "Application/Instruments/WavFileWriter.h"

class AudioDriver ;

#define MIX_BUFFER_SIZE 40000

class AudioOutDriver: public AudioOut,protected I_Observer {
public:
       AudioOutDriver(AudioDriver &) ;
       virtual ~AudioOutDriver() ;
 
       virtual bool Init() ;
       virtual void Close() ;
       virtual bool Start() ;
       virtual void Stop() ;
 
       virtual void Trigger() ;

       virtual bool Clipped() ;

	   virtual int GetPlayedBufferPercentage() ;

       AudioDriver *GetDriver() ;

		virtual std::string GetAudioAPI() ;
		virtual std::string GetAudioDevice() ;
		virtual int GetAudioBufferSize() ;
		virtual int GetAudioRequestedBufferSize() ;
		virtual int GetAudioPreBufferCount() ;
		virtual double GetStreamTime() ;

protected:

		virtual void Update(Observable &o,I_ObservableData *d) ;
 
       void prepareMixBuffers() ;
       void mixToPrimary() ;
	   void clipToMix() ;

private:
        AudioDriver *driver_ ;
       	bool clipped_ ;
       	bool hasSound_ ;

	    fixed *primarySoundBuffer_ ;
	    short *mixBuffer_ ;
	    int sampleCount_ ;       
} ;
#endif
