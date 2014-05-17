
#include "RTAudioStub.h"
#include "RTAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"
#include <map>


RTAudioStub::RTAudioStub(AudioSettings &h):Audio(h),
	sampleRate_(0) 
{

  std::map<int, std::string> apiMap;
  apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
  apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
  apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
  apiMap[RtAudio::UNIX_JACK] = "Jack Client";
  apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
  apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
  apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

  std::vector< RtAudio::Api > apis;
  RtAudio :: getCompiledApi( apis );
#ifdef __LINUX_ALSA__
  api_=RtAudio::LINUX_ALSA;
  if (!strcmp(GetAudioAPI(),"OSS")) {
	  api_=RtAudio::LINUX_OSS ; 
  } 
#else
#ifdef __MACOSX_CORE__
  api_=RtAudio::MACOSX_CORE;
#else
  api_=RtAudio::WINDOWS_DS;
  if (!strcmp(GetAudioAPI(),"ASIO")) {
	  api_=RtAudio::WINDOWS_ASIO ; 
  } 
#endif
#endif

  RtAudio audio(api_) ;


  Trace::Log("AUDIO","Current API: %s",apiMap[ audio.getCurrentApi()].c_str());

  unsigned int devices = audio.getDeviceCount();

  std::string defaultDevice ;

  std::string name=GetAudioDevice() ; 
  const char *deviceName=(name.size()!=0)?name.c_str():0 ;

  RtAudio::DeviceInfo selinfo ;
  for (unsigned int i=0; i<devices; i++) 
  {
    RtAudio::DeviceInfo info = audio.getDeviceInfo(i);
    if (info.outputChannels>0)
    {
      if (info.isDefaultOutput || defaultDevice.length() ==0)
      {
        defaultDevice=info.name ;
        if (!selinfo.probed) selinfo=info ;
      }

      if ((deviceName)&&(!strncmp(deviceName,info.name.c_str(),strlen(deviceName))))
      {
        selectedDevice_=info.name ;
        selinfo=info ;
      }
      Trace::Log("AUDIO","Found device %s",info.name.c_str()) ;
    }
  }
  if (selectedDevice_.length()==0)
  {
    selectedDevice_=defaultDevice ;
  }
  Trace::Log("AUDIO","Selecting: %s",selectedDevice_.c_str()) ;

/*  if ( selinfo.probed == false )
	  Trace::Debug("Probe Status = UNsuccessful\n");
    else {
      Trace::Debug("Probe Status = Successful\n");
      Trace::Debug("Output Channels = %d\n",selinfo.outputChannels);
      Trace::Debug("Input Channels = %d\n",selinfo.inputChannels);
      Trace::Debug("Duplex Channels = %d\n",selinfo.duplexChannels);
      if ( selinfo.isDefaultOutput ) Trace::Debug("This is the default output device.\n");
      else Trace::Debug("This is NOT the default output device.\n");
      if ( selinfo.isDefaultInput ) Trace::Debug("This is the default input device.\n");
      else Trace::Debug("This is NOT the default input device.\n");
      if ( selinfo.nativeFormats == 0 )
        Trace::Debug("No natively supported data formats(?)!");
      else {
        Trace::Debug("Natively supported data formats:\n");
        if ( selinfo.nativeFormats & RTAUDIO_SINT8 )
          Trace::Debug("  8-bit int\n");
        if ( selinfo.nativeFormats & RTAUDIO_SINT16 )
          Trace::Debug("  16-bit int\n");
        if ( selinfo.nativeFormats & RTAUDIO_SINT24 )
          Trace::Debug("  24-bit int\n");
        if ( selinfo.nativeFormats & RTAUDIO_SINT32 )
          Trace::Debug("  32-bit int\n");
        if ( selinfo.nativeFormats & RTAUDIO_FLOAT32 )
          Trace::Debug("  32-bit float\n");
        if ( selinfo.nativeFormats & RTAUDIO_FLOAT64 )
          Trace::Debug("  64-bit float\n");
      }
      if ( selinfo.sampleRates.size() < 1 )
        Trace::Debug("No supported sample rates found!");
      else {
        Trace::Debug("Supported sample rates = \n");
        for (unsigned int j=0; j<selinfo.sampleRates.size(); j++)
          Trace::Debug("%d\n",selinfo.sampleRates[j]);
      }
	}
		Trace::Debug("\n") ;	
	
*/	
}

RTAudioStub::~RTAudioStub() {
}

void RTAudioStub::Init() 
{
	AudioSettings settings ;
	settings.audioAPI_=GetAudioAPI();
	settings.audioDevice_=selectedDevice_;
	settings.bufferSize_=GetAudioBufferSize();
	settings.preBufferCount_=GetAudioPreBufferCount();

  RTAudioDriver *drv=new RTAudioDriver(api_,settings) ;
  AudioOutDriver *out=new AudioOutDriver(*drv) ;
  Insert(out) ;
  sampleRate_=drv->GetSampleRate() ;
}

void RTAudioStub::Close() 
{
  IteratorPtr<AudioOut>it(GetIterator()) ;
  for (it->Begin();!it->IsDone();it->Next())
  {
    AudioOut &current=it->CurrentItem() ;
    current.Close() ;
  }
}

int RTAudioStub::GetSampleRate()  
{
	return sampleRate_ ;
} ;
