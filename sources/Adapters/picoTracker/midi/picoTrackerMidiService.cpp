#include "picoTrackerMidiService.h"
#include "picoTrackerMidiInDevice.h"
#include "picoTrackerMidiOutDevice.h"
#include "picoTrackerUSBMidiDevice.h"
#include "picoTrackerUSBMidiInDevice.h"

picoTrackerMidiService::picoTrackerMidiService(){};

picoTrackerMidiService::~picoTrackerMidiService(){};

void picoTrackerMidiService::buildDriverList() {
  // create a midi device for each of Midi Output device
  MidiOutDevice *dev = new picoTrackerMidiOutDevice("MIDI OUT");
  outList_.insert(outList_.end(), dev);
  dev = new picoTrackerUSBMidiOutDevice("USB");
  outList_.insert(outList_.end(), dev);

  // Create MIDI input device
  MidiInDevice *inDev = new picoTrackerMidiInDevice("MIDI IN");
  inList_.insert(inList_.end(), inDev);

  // Add USB MIDI input device
  picoTrackerUSBMidiInDevice *usbMidiIn =
      new picoTrackerUSBMidiInDevice("USB MIDI IN");
  inList_.insert(inList_.end(), usbMidiIn);
};

void picoTrackerMidiService::poll() {
  // Poll all MIDI input devices
  for (auto dev : inList_) {
    picoTrackerMidiInDevice *ptDev = (picoTrackerMidiInDevice *)dev;
    if (ptDev) {
      ptDev->poll();
    }
  }
};
