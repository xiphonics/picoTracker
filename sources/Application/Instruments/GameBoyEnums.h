#pragma once

enum gbConstants { gbNumWaveforms = 8 };

enum gbWaveType {
  gbWavePulse12_5,
  gbWavePulse25,
  gbWavePulse50,
  gbWaveTriangle,
  gbWaveNoiseGameBoy,
  gbWaveNoiseNES,
  gbWaveNoiseSN76489,
  gbWaveNoiseWhite,
  gbWaveNone
};

enum gbEnvState { gbEnvIdle, gbEnvAttack, gbEnvDecay };

typedef union gbFlags {
  struct {
    uint8_t arpeggio : 1;
    uint8_t legato : 1;
    uint8_t retrigger : 1;
    uint8_t unused : 5;
  };
  uint8_t byte;
} gbFlags;