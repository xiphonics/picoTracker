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

MidiService::MidiService() : inList_(true), device_(0), sendSync_(true) {
  for (int i = 0; i < MIDI_MAX_BUFFERS; i++) {
    queues_[i].clear();
  }
  sendSync_ = Config::GetInstance()->GetValue("MIDISENDSYNC") > 0;
};

MidiService::~MidiService() { Close(); };

bool MidiService::Init() {
  outList_.Empty();
  inList_.Empty();
  buildDriverList();
  // Add a merger for the input
  merger_ = new MidiInMerger();
  for (inList_.Begin(); !inList_.IsDone(); inList_.Next()) {
    MidiInDevice &current = inList_.CurrentItem();
    merger_->Insert(current);
  }
  auto config = Config::GetInstance();
  auto midiDevVar =
      (WatchedVariable *)config->FindVariable(FourCC::VarMidiDevice);
  midiDevVar->AddObserver(*this);
  if (midiDevVar->GetInt() == 1) {
    // for now just hardcode to first and only TRS device until we enable USB
    device_ = outList_.GetFirst();
    device_->Init();
  }

  auto midiSyncVar =
      (WatchedVariable *)config->FindVariable(FourCC::VarMidiSync);
  midiSyncVar->AddObserver(*this);
  auto sync = midiSyncVar->GetInt();
  sendSync_ = sync != 0;

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
  if (device_) {
    auto queue = &queues_[currentPlayQueue_];
    queue->emplace_back(m.status_, m.data1_, m.data2_);
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
    auto deviceID = v.GetInt();
    // note deviceID has 0 == OFF
    printf("midi device var changed:%d", deviceID);

    if (deviceID == 0 && device_ != nullptr) {
      device_->Stop();
      device_->Close();
      device_ = nullptr;
    } else if (deviceID == 1) {
      // for now just hardcode to first and only TRS device until we enable USB
      device_ = outList_.GetFirst();
      device_->Init();
    }
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

  if (device_) {
    // Send whatever is on the out queue
    device_->SendQueue(*flushQueue);
  }
  flushQueue->clear();
}

void MidiService::startDevice() {
  // look for the device
  for (outList_.Begin(); !outList_.IsDone(); outList_.Next()) {
    MidiOutDevice &current = outList_.CurrentItem();
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
    stopDevice();
    startDevice();
    deviceName_ = "";
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
