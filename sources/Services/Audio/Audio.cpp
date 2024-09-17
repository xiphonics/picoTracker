#include "Audio.h"
#include "Application/Model/Config.h"

Audio::Audio(AudioSettings &hints) : T_SimpleList<AudioOut>(true), settings_() {

  // Hints contains the basic information about the
  // default settings for the platform. All of the can
  // be overriden through the config file

  Config *config = Config::GetInstance();
  settings_.audioAPI_ = hints.audioAPI_;
  settings_.audioDevice_ = hints.audioDevice_;

  Trace::Log("AUDIO", "Audio object initialised with");
  Trace::Log("AUDIO", "Api:%s", settings_.audioAPI_.c_str());
  Trace::Log("AUDIO", "Device:%s", settings_.audioDevice_.c_str());
  Trace::Log("AUDIO", "Buffer size:%d", settings_.bufferSize_);
  Trace::Log("AUDIO", "Pre Buffer Count:%d", settings_.preBufferCount_);
}

Audio::~Audio() {}

const char *Audio::GetAudioAPI() { return settings_.audioAPI_.c_str(); }

const char *Audio::GetAudioDevice() { return settings_.audioDevice_.c_str(); };

int Audio::GetAudioBufferSize() { return settings_.bufferSize_; };

int Audio::GetAudioPreBufferCount() { return settings_.preBufferCount_; };
