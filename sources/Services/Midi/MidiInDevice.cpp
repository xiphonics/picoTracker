#include "MidiInDevice.h"
#include "Application/Instruments/InstrumentBank.h"
#include "Application/Model/Config.h"
#include "Application/Player/Player.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"

#define MIDI_CHANNEL_MASK 0x0F
#define MIDI_DATA_MASK 0x7F

using namespace std;

// Set this to true to log MIDI events to stdout for debugging
bool MidiInDevice::dumpEvents_ = false;

// Initialize the static channel-to-instrument mapping array
int8_t MidiInDevice::channelToInstrument_[16] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

MidiInDevice::MidiInDevice(const char *name)
    : ControllerSource("midi", name), T_Stack<MidiMessage>(true) {
  for (int channel = 0; channel < 16; channel++) {
    // Initialize the new channel-to-instrument mapping
    channelToInstrument_[channel] = -1; // -1 means no instrument assigned
  }
  isRunning_ = false;
};

MidiInDevice::~MidiInDevice(){};

bool MidiInDevice::Init() { return initDriver(); };

void MidiInDevice::Close() { closeDriver(); };

bool MidiInDevice::Start() {
  isRunning_ = true;

  // Reset the MIDI parser state
  midiStatus = 0;
  midiDataCount = 0;

  return startDriver();
};

void MidiInDevice::Stop() {
  isRunning_ = false;
  stopDriver();
};

bool MidiInDevice::IsRunning() { return isRunning_; };

void MidiInDevice::onMidiStart() {
  MidiSyncData data(MSM_START);
  SetChanged();
  NotifyObservers();

  // Get the Player instance and start playback similar to SongView's onStart
  Player *player = Player::GetInstance();
  if (player) {
    // Start playback on all channels (0-7) like the play button does
    player->OnSongStartButton(0, 7, false, false);
  }
};

void MidiInDevice::onMidiStop() {
  MidiSyncData data(MSM_STOP);
  SetChanged();
  NotifyObservers();

  // Get the Player instance and stop playback similar to SongView's onStop
  Player *player = Player::GetInstance();
  if (player) {
    // Stop playback on all channels (0-7)
    player->OnSongStartButton(0, 7, true, false);
  }
};

void MidiInDevice::onMidiTempoTick() {
  MidiSyncData data(MSM_TEMPOTICK);
  SetChanged();
  NotifyObservers();
};

void MidiInDevice::queueEvent(MidiEvent &event){
    // TODO: queue the event
};

void MidiInDevice::onDriverMessage(MidiMessage &message) {
  SetChanged();
  NotifyObservers(&message);
  treatChannelEvent(message);
};

void MidiInDevice::Trigger(Time time) {

  MidiMessage *event = Pop(true);
  while (event) {
    treatChannelEvent(*event);
    delete event;

    event = Pop(true);
  };

  for (int midiChannel = 0; midiChannel < 16; midiChannel++) {
    // TODO: Trigger all channels with relevant midi messages
  }
};

void MidiInDevice::treatChannelEvent(MidiMessage &event) {

  int midiChannel = event.status_ & MIDI_CHANNEL_MASK;

  bool isMidiClockEvent = (event.status_ == MidiMessage::MIDI_CLOCK);

  // display as hex
  if (!isMidiClockEvent) {
    Trace::Debug("midi:%02X:%02X:%02X", event.status_, event.data1_,
                 event.data2_);
    Trace::Debug("miditype:%02X", event.GetType());
  }

  // First check for system real-time messages which need to be compared with
  // the full status byte
  if (event.status_ == MidiMessage::MIDI_CLOCK) {
    onMidiTempoTick();
    return;
  } else if (event.status_ == MidiMessage::MIDI_START) {
    onMidiStart();
    return;
  } else if (event.status_ == MidiMessage::MIDI_STOP) {
    onMidiStop();
    return;
  } else if (event.status_ == MidiMessage::MIDI_CONTINUE) {
    // TODO: how to handle continue message as pT doesnt have concept of pausing
    // then continuing playback from the same position
    return;
  }

  // Process channel messages using GetType()
  switch (event.GetType()) {
  case MidiMessage::MIDI_NOTE_OFF: {
    int note = event.data1_ & MIDI_DATA_MASK;

    // Check if we have a direct instrument mapping for this channel
    if (channelToInstrument_[midiChannel] != -1) {
      int instrumentIndex = channelToInstrument_[midiChannel];
      Player *player = Player::GetInstance();
      if (player) {
        player->StopNote(instrumentIndex, midiChannel);
      }
    }
  } break;

  case MidiMessage::MIDI_NOTE_ON: {
    int note = event.data1_ & MIDI_DATA_MASK;
    int value = event.data2_ & MIDI_DATA_MASK;

    // Check if we have a direct instrument mapping for this channel
    if (channelToInstrument_[midiChannel] != -1) {
      short instrumentIndex = channelToInstrument_[midiChannel];
      Player *player = Player::GetInstance();
      if (player) {
        // If velocity is 0, it's actually a note off in MIDI
        if (value == 0) {
          player->StopNote(instrumentIndex, midiChannel);
        } else {
          player->PlayNote(instrumentIndex, midiChannel, note, value);
        }
      }
    } else {
      Trace::Debug("No instrument assigned for MIDI channel %d", midiChannel);
    }
  } break;

  case MidiMessage::MIDI_AFTERTOUCH: {
    int note = event.data1_ & MIDI_DATA_MASK;
    int data = event.data2_ & MIDI_DATA_MASK;

    // TODO: handle aftertouch
  } break;

  case MidiMessage::MIDI_CONTROL_CHANGE: {
    int cc = event.data1_ & MIDI_DATA_MASK;
    int data = event.data2_ & MIDI_DATA_MASK;

    if (dumpEvents_) {
      Trace::Log("EVENT", "midi:cc:%d:%d", cc, data);
    }
    // TODO: handle CC
  } break;

  case MidiMessage::MIDI_PROGRAM_CHANGE: {
    int data = event.data1_ & MIDI_DATA_MASK;

    // TODO: handle program change
  } break;

  case MidiMessage::MIDI_CHANNEL_AFTERTOUCH: {
    int data = event.data1_ & MIDI_DATA_MASK;

    // TODO: handle channel aftertouch
  } break;

  case MidiMessage::MIDI_PITCH_BEND: {
    // TODO: handle pitch bend
  } break;

  default:
    break;
  };
};

// New methods for direct instrument mapping
void MidiInDevice::AssignInstrumentToChannel(int midiChannel,
                                             int instrumentIndex) {
  if (midiChannel >= 0 && midiChannel < 16) {
    channelToInstrument_[midiChannel] = instrumentIndex;
    Trace::Log("MIDI", "Assigned instrument %d to MIDI channel %d",
               instrumentIndex, midiChannel);
  }
}

int MidiInDevice::GetInstrumentForChannel(int midiChannel) {
  if (midiChannel >= 0 && midiChannel < 16) {
    return channelToInstrument_[midiChannel];
  }
  return -1; // No instrument assigned
}

void MidiInDevice::ClearChannelAssignment(int midiChannel) {
  if (midiChannel >= 0 && midiChannel < 16) {
    channelToInstrument_[midiChannel] = -1;
  }
}

Channel *MidiInDevice::GetChannel(const char *sourcePath) {

  Channel *channel = 0;

  string path = sourcePath;
  string::size_type pos = path.find(":", 0);
  if (pos == string::npos) {
    return 0;
  };

  MidiChannel **ccChannel = 0;
  MidiChannel **noteChannel = 0;
  MidiChannel **pbChannel = 0;
  MidiChannel **pcChannel = 0;
  MidiChannel **catChannel = 0;
  MidiChannel **atChannel = 0;
  MidiChannel **activityChannel = 0;

  string firstElem = path.substr(0, pos);
  string type = "";

  // MIDI channel dependent channels

  string &midiChannelStr = firstElem;

  // First read the channel number

  int midiChannel = atoi(midiChannelStr.c_str());
  if ((midiChannel < 0) || (midiChannel > 15)) {
    return 0;
  };
  ;

  // read type
  path = path.substr(pos + 1);
  pos = path.find(":", 0);
  type = path.substr(0, pos);

  // Read the event id (note, cc, pb)
  pos = path.find(":", 0);
  path = path.substr(pos + 1);
  int id = atoi(path.c_str());
  if ((id < 0) || (id > 127)) {
    return 0;
  };

  if (type.substr(0, 2) == "cc") {

    MidiControllerType ccType = MCT_NONE;
    bool isCircular = false;
    bool isHiRes = false;

    if (type[2] != 0) {
      switch (type[2]) {
      case L'+':
        ccType = MCT_2_COMP;
        break;
      case L'|':
        ccType = MCT_HIRES;
        isHiRes = true;
        break;
      case L'-':
        ccType = MCT_SIGNED_BIT;
        break;
      case L'_':
        ccType = MCT_SIGNED_BIT_2;
        break;
      case L'=':
        ccType = MCT_BIN_OFFSET;
        break;
      case L'*': // backward compatibility
        ccType = MCT_2_COMP;
        isCircular = true;
        // assert(0) ;
        break;
      }
      if (type[3] != 0) {
        NAssert(type[3] == L'*');
        isCircular = true;
      };
    };

    if (*ccChannel == 0) {
      *ccChannel = new MidiChannel(sourcePath);
    }
    (*ccChannel)->SetControllerType(ccType);
    (*ccChannel)->SetCircular(isCircular);
    (*ccChannel)->SetHiRes(isHiRes);
    channel = *ccChannel;
  };

  if (type == "note") {
    if (*noteChannel == 0) {
      *noteChannel = new MidiChannel(sourcePath);
    };
    channel = *noteChannel;
  };
  if (type == "note+") {
    if (*noteChannel == 0) {
      *noteChannel = new MidiChannel(sourcePath);
      (*noteChannel)->SetToggle(true);
    };
    channel = *noteChannel;
  };
  if (type == "at") {
    if (*atChannel == 0) {
      (*atChannel) = new MidiChannel(sourcePath);
    };
    channel = *atChannel;
  };
  if (type == "pb") {
    if (*pbChannel == 0) {
      *pbChannel = new MidiChannel(sourcePath);
    };
    channel = *pbChannel;
  };
  if (type == "cat") {
    if (*catChannel == 0) {
      *catChannel = new MidiChannel(sourcePath);
    };
    channel = *catChannel;
  };
  if (type == "pc") {
    if (*catChannel == 0) {
      *catChannel = new MidiChannel(sourcePath);
    };
    channel = *pcChannel;
  };
  if (type == "activity") {
    if (*activityChannel == 0) {
      *activityChannel = new MidiChannel(sourcePath);
    };
    channel = *activityChannel;
  };
  return channel;
  ;
};

void MidiInDevice::treatCC(MidiChannel *channel, int data, bool hiNibble) {
  switch (channel->GetControllerType()) {
  // Regular midi channels
  case MCT_NONE: // to cope with the fact we want to have the possibility to map
                 // MIDI controllers to 0.5,0.25, etc, we map differently if
                 // data is zero or otherwise.

    channel->SetValue(
        (data > 0) ? float(data + 1) / (channel->GetRange() / 2.0f) : 0);
    break;
  case MCT_HIRES: {
    float channelValue = channel->GetValue();
    int current = (channelValue == 0) ? 0 : int(channelValue * 16384 - 1);
    int hi = current / 128;
    int low = current - hi * 128;
    if (hiNibble) {
      hi = data;
    } else {
      low = data;
    }
    current = hi * 128 + low;
    channel->SetValue((current == 0) ? 0 : (current + 1) / 16384.0f);
  } break;
  case MCT_2_COMP: {
    float current = channel->GetValue();
    int incr = 0;
    if (data != 0) {
      if (data < 0x41) {
        incr = data;
      } else {
        incr = data - 0x80;
      };
    };
    current += float(incr) / (channel->GetRange() / 2.0f - 1.0f);
    if (channel->IsCircular()) {
      if (current > 1.0)
        current -= 1.0F;
      if (current < 0.0)
        current += 1.0F;
    } else {
      if (current > 1.0)
        current = 1.0F;
      if (current < 0.0)
        current = 0.0F;
    }
    channel->SetValue(current);
    break;
  }
  case MCT_SIGNED_BIT: {
    float current = channel->GetValue();
    int incr = 0;
    if (data != 0) {
      if (data < 0x41) {
        incr = data;
      } else {
        incr = 0x40 - data;
      };
    };
    current += float(incr) / (channel->GetRange() / 2.0f - 1.0f);
    if (channel->IsCircular()) {
      if (current > 1.0)
        current -= 1.0F;
      if (current < 0.0)
        current += 1.0F;
    } else {
      if (current > 1.0)
        current = 1.0F;
      if (current < 0.0)
        current = 0.0F;
    }
    channel->SetValue(current);
    break;
  }
  case MCT_SIGNED_BIT_2: {
    float current = channel->GetValue();
    int incr = 0;
    if (data != 0) {
      if (data < 0x41) {
        incr = -data;
      } else {
        incr = data - 0x40;
      };
    };
    current += float(incr) / (channel->GetRange() / 2.0f - 1.0f);
    if (channel->IsCircular()) {
      if (current > 1.0)
        current -= 1.0F;
      if (current < 0.0)
        current += 1.0F;
    } else {
      if (current > 1.0)
        current = 1.0F;
      if (current < 0.0)
        current = 0.0F;
    }
    channel->SetValue(current);
    break;
  }
  case MCT_BIN_OFFSET: {
    float current = channel->GetValue();
    int incr = 0;
    if (data != 0) {
      if (data < 0x41) {
        incr = data - 0x40;
      } else {
        incr = data - 0x40;
      };
    };
    current += float(incr) / (channel->GetRange() / 2.0f - 1.0f);
    if (channel->IsCircular()) {
      if (current > 1.0)
        current -= 1.0F;
      if (current < 0.0)
        current += 1.0F;
    } else {
      if (current > 1.0)
        current = 1.0F;
      if (current < 0.0)
        current = 0.0F;
    }
    channel->SetValue(current);
    break;
  }
  };
  channel->Trigger();
};

void MidiInDevice::treatNoteOff(MidiChannel *channel) {
  if (!channel->IsToggle()) {
    channel->SetValue(0.0F);
  }
  channel->Trigger();
};

void MidiInDevice::treatNoteOn(MidiChannel *channel, int value) {
  if (value == 0) { // Actually a note off
    if (!channel->IsToggle()) {
      channel->SetValue(0.0F);
    }
  } else {
    if (!channel->IsToggle()) {
      channel->SetValue(1.0F);
    } else {
      float current = channel->GetValue();
      channel->SetValue((current > 0.5) ? 0.0f : 1.0f);
    };
  }
  channel->Trigger();
};

void MidiInDevice::processMidiData(uint8_t data) {
  // Handle MIDI data byte
  if (data & 0x80) {
    // This is a status byte

    // System Real-Time messages can appear anywhere and have no data bytes
    if (data >= 0xF8) {
      MidiMessage msg;
      msg.status_ = data;
      msg.data1_ = MidiMessage::UNUSED_BYTE;
      msg.data2_ = MidiMessage::UNUSED_BYTE;
      onDriverMessage(msg);
      return; // Don't change the running status
    }

    // For all other status bytes, reset the data count
    midiStatus = data;
    midiDataCount = 0;

    // Determine how many data bytes to expect based solely on the status byte
    if (data >= 0xF0) {
      // System Common messages
      switch (data) {
      case 0xF1: // MIDI Time Code Quarter Frame
      case 0xF3: // Song Select
        midiDataBytes = 1;
        break;
      case 0xF2: // Song Position Pointer
        midiDataBytes = 2;
        break;
      case 0xF0: // Start of System Exclusive
        // SysEx messages are variable length and end with 0xF7
        // For simplicity, we're not fully handling SysEx here
        midiDataBytes = 0; // Special case, handled differently
        break;
      default:
        // All other System Common messages have no data bytes
        midiDataBytes = 0;

        // For messages with no data bytes, send them immediately
        MidiMessage msg;
        msg.status_ = midiStatus;
        msg.data1_ = MidiMessage::UNUSED_BYTE;
        msg.data2_ = MidiMessage::UNUSED_BYTE;
        onDriverMessage(msg);
        break;
      }
    } else {
      // Channel messages - determine bytes by status byte range
      uint8_t msgType = data & 0xF0;

      // Program Change and Channel Pressure have 1 data byte
      if (msgType == 0xC0 || msgType == 0xD0) {
        midiDataBytes = 1;
      } else {
        // All other channel messages have 2 data bytes
        midiDataBytes = 2;
      }
    }
  } else {
    // This is a data byte
    if (midiStatus == 0) {
      // Ignore data bytes without status
      Trace::Debug("MIDI", "Ignored data byte without status: 0x%02X", data);
      return;
    }

    if (midiDataCount == 0) {
      midiData1 = data;
      midiDataCount++;

      // If we only expect one data byte, we have a complete message
      if (midiDataBytes == 1) {
        MidiMessage msg;
        msg.status_ = midiStatus;
        msg.data1_ = midiData1;
        msg.data2_ = MidiMessage::UNUSED_BYTE;
        onDriverMessage(msg);
      }
    } else if (midiDataCount == 1 && midiDataBytes == 2) {
      // We have all the data we need for a 2-byte message
      MidiMessage msg;
      msg.status_ = midiStatus;
      msg.data1_ = midiData1;
      msg.data2_ = data;
      onDriverMessage(msg);

      // Reset data count but keep status for running status
      midiDataCount = 0;
    }
  }
}