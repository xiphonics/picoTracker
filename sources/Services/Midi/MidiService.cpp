#include "MidiService.h"
#include "Application/Model/Config.h"
#include "Application/Player/Player.h"
#include "Application/Player/SyncMaster.h"
#include "Services/Audio/AudioDriver.h"
#include "System/Console/Trace.h"
#include "System/Timer/Timer.h"
#include <cstring>

#ifdef SendMessage
#undef SendMessage
#endif

MidiService::MidiService() : sendSync_(true) {
  for (int i = 0; i < MIDI_MAX_BUFFERS; i++) {
    queues_[i].clear();
  }
  sendSync_ = Config::GetInstance()->GetValue("MIDISENDSYNC") > 0;
};

MidiService::~MidiService() { Close(); };

bool MidiService::Init() {
  outList_.empty();
  inList_.empty();
  buildDriverList();
  // Init all the output midi devices
  for (auto dev : outList_) {
    dev->Init();
  }

  // Add a merger for the input
  merger_ = new MidiInMerger();
  for (auto dev : inList_) {
    merger_->Insert(*dev);
    dev->Start();
    // Add this service as an observer to receive transport control messages
    dev->AddObserver(*this);
  }

  // Initialize the new channel-to-instrument mapping
  for (short i = 0; i < 16; i++) {
    inList_[0]->AssignInstrumentToChannel(i, i);
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

void MidiService::OnMidiStart() {
  // Start the Player in song mode
  Trace::Log("MIDI", "Received MIDI Start message");
  Player::GetInstance()->Start(PM_SONG, true);
}

void MidiService::OnMidiStop() {
  // Stop the Player
  Trace::Log("MIDI", "Received MIDI Stop message");
  Player::GetInstance()->Stop();
}

void MidiService::OnMidiClock() {
  // Handle MIDI clock messages if needed
  // This could be used for tempo synchronization
}

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
  // Stop and restart devices to ensure clean state
  stopDevice();
  startDevice();

  // Send MIDI Start message (0xFA) to all active output devices
  if (sendSync_) {
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_START;
    QueueMessage(msg);
  }
};

void MidiService::OnPlayerStop() {
  // Send MIDI Stop message (0xFC) to all active output devices
  if (sendSync_) {
    MidiMessage msg;
    msg.status_ = MidiMessage::MIDI_STOP;
    QueueMessage(msg);
  }
};
