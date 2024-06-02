// This is the Opal OPL3 emulator from Reality Adlib Tracker v2.0a
// (http://www.3eality.com/productions/reality-adlib-tracker). It was released
// by Shayde/Reality into the public domain. Minor modifications to silence some
// warnings and fix a bug in the envelope generator have been applied.
// Additional fixes by JP Cimalando.

/*

    The Opal OPL3 emulator.

    Note: this is not a complete emulator, just enough for Reality Adlib Tracker
   tunes.

    Missing features compared to a real OPL3:

        - Timers/interrupts
        - OPL3 enable bit (it defaults to always on)
        - CSW mode
        - Test register
        - Percussion mode

*/

#ifndef _OPAL_H_
#define _OPAL_H_

#include <cstdint>

//==================================================================================================
// Opal class.
//==================================================================================================
class Opal {

  class Channel;

  // Various constants
  enum {
    OPL3SampleRate = 49716,
    NumChannels = 18,
    NumOperators = 36,

    EnvOff = -1,
    EnvAtt,
    EnvDec,
    EnvSus,
    EnvRel,
  };

  // A single FM operator
  class Operator {

  public:
    Operator();
    void SetMaster(Opal *opal) { Master = opal; }
    void SetChannel(Channel *chan) { Chan = chan; }

    int16_t Output(uint16_t keyscalenum, uint32_t phase_step, int16_t vibrato,
                   int16_t mod = 0, int16_t fbshift = 0);

    void SetKeyOn(bool on);
    void SetTremoloEnable(bool on);
    void SetVibratoEnable(bool on);
    void SetSustainMode(bool on);
    void SetEnvelopeScaling(bool on);
    void SetFrequencyMultiplier(uint16_t scale);
    void SetKeyScale(uint16_t scale);
    void SetOutputLevel(uint16_t level);
    void SetAttackRate(uint16_t rate);
    void SetDecayRate(uint16_t rate);
    void SetSustainLevel(uint16_t level);
    void SetReleaseRate(uint16_t rate);
    void SetWaveform(uint16_t wave);

    void ComputeRates();
    void ComputeKeyScaleLevel();

  protected:
    Opal *Master;            // Master object
    Channel *Chan;           // Owning channel
    uint32_t Phase;          // The current offset in the selected waveform
    uint16_t Waveform;       // The waveform id this operator is using
    uint16_t FreqMultTimes2; // Frequency multiplier * 2
    int EnvelopeStage; // Which stage the envelope is at (see Env* enums above)
    int16_t EnvelopeLevel; // 0 - $1FF, 0 being the loudest
    uint16_t OutputLevel;  // 0 - $FF
    uint16_t AttackRate;
    uint16_t DecayRate;
    uint16_t SustainLevel;
    uint16_t ReleaseRate;
    uint16_t AttackShift;
    uint16_t AttackMask;
    uint16_t AttackAdd;
    const uint16_t *AttackTab;
    uint16_t DecayShift;
    uint16_t DecayMask;
    uint16_t DecayAdd;
    const uint16_t *DecayTab;
    uint16_t ReleaseShift;
    uint16_t ReleaseMask;
    uint16_t ReleaseAdd;
    const uint16_t *ReleaseTab;
    uint16_t KeyScaleShift;
    uint16_t KeyScaleLevel;
    int16_t Out[2];
    bool KeyOn;
    bool KeyScaleRate; // Affects envelope rate scaling
    bool SustainMode; // Whether to sustain during the sustain phase, or release
                      // instead
    bool TremoloEnable;
    bool VibratoEnable;
  };

  // A single channel, which can contain two or more operators
  class Channel {

  public:
    Channel();
    void SetMaster(Opal *opal) { Master = opal; }
    void SetOperators(Operator *a, Operator *b, Operator *c, Operator *d) {
      Op[0] = a;
      Op[1] = b;
      Op[2] = c;
      Op[3] = d;
      if (a)
        a->SetChannel(this);
      if (b)
        b->SetChannel(this);
      if (c)
        c->SetChannel(this);
      if (d)
        d->SetChannel(this);
    }

    void Output(int16_t &left, int16_t &right);
    void SetEnable(bool on) { Enable = on; }
    void SetChannelPair(Channel *pair) { ChannelPair = pair; }

    void SetFrequencyLow(uint16_t freq);
    void SetFrequencyHigh(uint16_t freq);
    void SetKeyOn(bool on);
    void SetOctave(uint16_t oct);
    void SetLeftEnable(bool on);
    void SetRightEnable(bool on);
    void SetFeedback(uint16_t val);
    void SetModulationType(uint16_t type);

    uint16_t GetFreq() const { return Freq; }
    uint16_t GetOctave() const { return Octave; }
    uint16_t GetKeyScaleNumber() const { return KeyScaleNumber; }
    uint16_t GetModulationType() const { return ModulationType; }
    Channel *GetChannelPair() const { return ChannelPair; }

    void ComputeKeyScaleNumber();

  protected:
    void ComputePhaseStep();

    Operator *Op[4];

    Opal *Master;    // Master object
    uint16_t Freq;   // Frequency; actually it's a phase stepping value
    uint16_t Octave; // Also known as "block" in Yamaha parlance
    uint32_t PhaseStep;
    uint16_t KeyScaleNumber;
    uint16_t FeedbackShift;
    uint16_t ModulationType;
    Channel *ChannelPair;
    bool Enable;
    bool LeftEnable, RightEnable;
  };

public:
  Opal(int sample_rate);
  Opal(const Opal &) = delete;
  Opal(Opal &&) = delete;
  ~Opal();

  void SetSampleRate(int sample_rate);
  void Port(uint16_t reg_num, uint8_t val);
  void Sample(int16_t *left, int16_t *right);

protected:
  void Init(int sample_rate);
  void Output(int16_t &left, int16_t &right);

  int32_t SampleRate;
  int32_t SampleAccum;
  int16_t LastOutput[2], CurrOutput[2];
  Channel Chan[NumChannels];
  Operator Op[NumOperators];
  //      uint16_t            ExpTable[256];
  //      uint16_t            LogSinTable[256];
  uint16_t Clock;
  uint16_t TremoloClock;
  uint16_t TremoloLevel;
  uint16_t VibratoTick;
  uint16_t VibratoClock;
  bool NoteSel;
  bool TremoloDepth;
  bool VibratoDepth;

  static const uint16_t RateTables[4][8];
  static const uint16_t ExpTable[256];
  static const uint16_t LogSinTable[256];
};

#endif
