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

MidiService::MidiService() : inList_(true), sendSync_(true) {
  for (int i = 0; i < MIDI_MAX_BUFFERS; i++) {
    queues_[i].clear();
  }
  sendSync_ = Config::GetInstance()->GetValue("MIDISENDSYNC") > 0;
};

MidiService::~MidiService() { Close(); };

bool MidiService::Init() {
  outList_.empty();
  inList_.Empty();
  buildDriverList();
  // Init all the output midi devices
  for (auto dev : outList_) {
    dev->Init();
  }

  // Add a merger for the input
  merger_ = new MidiInMerger();
  for (inList_.Begin(); !inList_.IsDone(); inList_.Next()) {
    MidiInDevice &current = inList_.CurrentItem();
    merger_->Insert(current);
  }

#ifndef DUMMY_MIDI
  auto config = Config::GetInstance();
  auto midiDevVar =
      (WatchedVariable *)config->FindVariable(FourCC::VarMidiDevice);
  midiDevVar->AddObserver(*this);

  auto activeDeviceConfig = midiDevVar->GetInt();
  updateActiveDevicesList(activeDeviceConfig);

  auto midiSyncVar =
      (WatchedVariable *)config->FindVariable(FourCC::VarMidiSync);
  midiSyncVar->AddObserver(*this);
  auto sync = midiSyncVar->GetInt();
  sendSync_ = sync != 0;
#endif

  return true;
};

void MidiService::Close() { Stop(); };

bool MidiService::Start() {
  currentPlayQueue_ = 0;
  currentOutQueue_ = 0;
  return true;
};

void MidiService::Stop() { stopDevice(); };

void MidiService::QueueMessage(MidiMessage &m) {
  if (!activeOutDevices_.empty()) {
    auto queue = &queues_[currentPlayQueue_];
    queue->emplace_back(m.status_, m.data1_, m.data2_);
  }
};

void MidiService::Trigger() {
  AdvancePlayQueue();

  if (!activeOutDevices_.empty() && sendSync_) {
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
  auto queue = &queues_[currentPlayQueue_];
  queue->clear();
}

void MidiService::Update(Observable &o, I_ObservableData *d) {
  AudioDriver::Event *event = (AudioDriver::Event *)d;
  if (event->type_ == AudioDriver::Event::ADET_DRIVERTICK) {
    onAudioTick();
  }
  WatchedVariable &v = (WatchedVariable &)o;
  switch (v.GetID()) {
    // need braces inside case statements due to:
    // https://stackoverflow.com/a/11578973/85472
  case FourCC::VarMidiDevice: {
    auto activeDeviceConfig = v.GetInt();
    // note deviceID has 0 == OFF
    Trace::Debug("midi device var changed:%d", activeDeviceConfig);

    stopDevice();
    updateActiveDevicesList(activeDeviceConfig);
    startDevice();
  } break;
  case FourCC::VarMidiSync: {
    auto sync = v.GetInt();
    sendSync_ = sync != 0;
  } break;
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
  auto flushQueue = &queues_[currentOutQueue_];

  for (auto dev : activeOutDevices_) {
    dev->SendQueue(*flushQueue);
  }
  flushQueue->clear();
}

void MidiService::updateActiveDevicesList(unsigned short config) {
  activeOutDevices_.clear();

  switch (config) {
  case 1:
    activeOutDevices_.insert(activeOutDevices_.end(), outList_[0]);
    break;
  case 2:
    activeOutDevices_.insert(activeOutDevices_.end(), outList_[1]);
    break;
  case 3:
    activeOutDevices_.insert(activeOutDevices_.end(), outList_[0]);
    activeOutDevices_.insert(activeOutDevices_.end(), outList_[1]);
    break;
  }
}

void MidiService::startDevice() {
  // look for the device
  for (auto dev : activeOutDevices_) {
    auto name = dev->GetName();
    dev->Start();
  }
};

void MidiService::stopDevice() {
  for (auto dev : outList_) {
    dev->Stop();
  }
};

void MidiService::OnPlayerStart() {
  for (auto dev : activeOutDevices_) {
    stopDevice();
    startDevice();

    if (sendSync_) {
      MidiMessage msg;
      msg.status_ = 0xFA;
      QueueMessage(msg);
    }
  }
};

void MidiService::OnPlayerStop() {
  if (sendSync_) {
    MidiMessage msg;
    msg.status_ = 0xFC;
    QueueMessage(msg);
  }
};
