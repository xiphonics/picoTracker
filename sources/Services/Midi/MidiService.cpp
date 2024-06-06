#include "MidiService.h"
#include "Application/Model/Config.h"
#include "Application/Player/SyncMaster.h"
#include "Services/Audio/AudioDriver.h"
#include "System/Console/Trace.h"
#include "System/Timer/Timer.h"
#include <cstring>

#ifdef SendMessage
#undef SendMessage
#endif

MidiService::MidiService()
    : T_SimpleList<MidiOutDevice>(true), inList_(true), device_(0),
      sendSync_(true) {
  for (int i = 0; i < MIDI_MAX_BUFFERS; i++) {
    queues_[i] = new T_SimpleList<MidiMessage>(true);
  }

  const char *delay = Config::GetInstance()->GetValue("MIDIDELAY");
  midiDelay_ = delay ? atoi(delay) : 1;

  const char *sendSync = Config::GetInstance()->GetValue("MIDISENDSYNC");
  if (sendSync) {
    sendSync_ = (strcmp(sendSync, "YES") == 0);
  }
};

MidiService::~MidiService() { Close(); };

bool MidiService::Init() {
  Empty();
  inList_.Empty();
  buildDriverList();
  // Add a merger for the input
  merger_ = new MidiInMerger();
  for (inList_.Begin(); !inList_.IsDone(); inList_.Next()) {
    MidiInDevice &current = inList_.CurrentItem();
    merger_->Insert(current);
  }

  return true;
};

void MidiService::Close() { Stop(); };

void MidiService::SelectDevice(const std::string &name) { deviceName_ = name; };

bool MidiService::Start() {
  currentPlayQueue_ = 0;
  currentOutQueue_ = 0;
  return true;
};

void MidiService::Stop() { stopDevice(); };

void MidiService::QueueMessage(MidiMessage &m) {
  if (device_) {
    T_SimpleList<MidiMessage> *queue = queues_[currentPlayQueue_];
    MidiMessage *ms = new MidiMessage(m.status_, m.data1_, m.data2_);
    queue->Insert(ms);
  }
};

void MidiService::Trigger() {
  AdvancePlayQueue();

  if (device_ && sendSync_) {
    SyncMaster *sm = SyncMaster::GetInstance();
    if (sm->MidiSlice()) {
      MidiMessage msg;
      msg.status_ = 0xF8;
      QueueMessage(msg);
    }
  }
}

void MidiService::AdvancePlayQueue() {
  currentPlayQueue_ = (currentPlayQueue_ + 1) % MIDI_MAX_BUFFERS;
  T_SimpleList<MidiMessage> *queue = queues_[currentPlayQueue_];
  queue->Empty();
}

void MidiService::Update(Observable &o, I_ObservableData *d) {
  AudioDriver::Event *event = (AudioDriver::Event *)d;
  if (event->type_ == AudioDriver::Event::ADET_DRIVERTICK) {
    onAudioTick();
  }
}

void MidiService::onAudioTick() {
  if (tickToFlush_ > 0) {
    if (--tickToFlush_ == 0) {
      flushOutQueue();
    }
  }
}

void MidiService::Flush() {

  tickToFlush_ = midiDelay_;
  if (tickToFlush_ == 0) {
    flushOutQueue();
  }
};

void MidiService::flushOutQueue() {
  // Move queue positions
  currentOutQueue_ = (currentOutQueue_ + 1) % MIDI_MAX_BUFFERS;
  T_SimpleList<MidiMessage> *flushQueue = queues_[currentOutQueue_];

  if (device_) {
    // Send whatever is on the out queue
    device_->SendQueue(*flushQueue);
  }
  flushQueue->Empty();
}

void MidiService::startDevice() {
  // loop through and init+start all midi devices
  for (Begin(); !IsDone(); Next()) {
    MidiOutDevice &current = CurrentItem();
    if (!strcmp(deviceName_.c_str(), current.GetName())) {
      if (current.Init()) {
        if (current.Start()) {
          device_ = &current;
        } else {
          current.Close();
        }
      }
      break;
    }
  }
};

void MidiService::stopDevice() {
  if (device_) {
    device_->Stop();
    device_->Close();
  }
  device_ = 0;
};

void MidiService::OnPlayerStart() {

  if (deviceName_.size() != 0) {
    // printf("ReStart Midi DEVICE NAME:%s", deviceName_.c_str());
    stopDevice();
    startDevice();
    deviceName_ = "MIDI OUT 1";
  } else {
    // printf("Start Default Pico Midi out device");
    startDevice();
    deviceName_ = "MIDI OUT 1";
  }

  if (sendSync_) {
    MidiMessage msg;
    msg.status_ = 0xFA;
    QueueMessage(msg);
  }
};

void MidiService::OnPlayerStop() {

  if (sendSync_) {
    MidiMessage msg;
    msg.status_ = 0xFC;
    QueueMessage(msg);
  }
};
