/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SRP_UPDATERS_H_
#define _SRP_UPDATERS_H_

#include "Foundation/Types/Types.h"
#include "I_SRPUpdater.h"

class VolumeRamp : public I_SRPUpdater {
public:
  VolumeRamp(){};
  virtual ~VolumeRamp(){};
  void SetData(float target, float speed, float start);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  fixed target_;
  fixed speed_;
};

class FCRamp : public I_SRPUpdater {
public:
  FCRamp(){};
  virtual ~FCRamp(){};
  void SetData(float target, float speed, float start);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  fixed target_;
  fixed speed_;
};

class FRRamp : public I_SRPUpdater {
public:
  FRRamp(){};
  virtual ~FRRamp(){};
  void SetData(float target, float speed, float start);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  fixed target_;
  fixed speed_;
};

class LogSpeedRamp : public I_SRPUpdater {
public:
  LogSpeedRamp(){};
  virtual ~LogSpeedRamp(){};
  void SetData(float target, float speed, float start);
  float GetCurrent();
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  fixed target_;
  fixed speed_;
};

class LinSpeedRamp : public I_SRPUpdater {
public:
  LinSpeedRamp(){};
  virtual ~LinSpeedRamp(){};
  void SetData(float target, float speed, float start);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  fixed target_;
  fixed speed_;
};

class Arp : public I_SRPUpdater {
public:
  Arp(){};
  virtual ~Arp(){};
  void SetData(uint data);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  uchar arp_[5];      // Arp setting
  uchar arpPosition_; // Position of in the arpegiator
  uchar arpLength_;   // Length of arp data
  fixed current_;
};

class Panner : public I_SRPUpdater {
public:
  Panner(){};
  virtual ~Panner(){};
  void SetData(float target, float speed, float start);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  fixed target_;
  fixed speed_;
};

class Vibrato : public I_SRPUpdater {
public:
  Vibrato(){};
  virtual ~Vibrato(){};
  void SetData(uint8_t rate, uint8_t depth);
  virtual void Trigger(bool tableTick);
  virtual void UpdateSRP(struct RUParams &rup);

private:
  fixed current_;
  uint8_t depth_;
  uint16_t phase_;
  uint16_t rate_;
};

#endif
