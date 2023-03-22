//cRSID SID emulation engine
#include "SID.h"
#include "SIDfilter.h"
#include "SIDwaves.h"

int cRSID::cRSID_emulateSIDoutputStage () {
  enum SIDspecs {
    CHANNELS = 3 + 1,
    VOLUME_MAX = 0xF,
    D418_DIGI_VOLUME = 2
  }; // digi-channel is counted too
  enum FilterBits {
    OFF3_BITVAL = 0x80,
    HIGHPASS_BITVAL = 0x40,
    BANDPASS_BITVAL = 0x20,
    LOWPASS_BITVAL = 0x10
  };

  static char MainVolume;
  static unsigned char FilterSwitchReso, VolumeBand;
  static int Tmp, NonFilted, FilterInput, Cutoff, ResonancePos, FilterOutput,
      Output;
  unsigned short Resonance;

  FilterSwitchReso = Register[0x17];
  VolumeBand = Register[0x18];
  Cutoff = (Register[0x16] << 3) + (Register[0x15] & 7);
  ResonancePos = FilterSwitchReso >> 4;

  NonFilted = NonFiltedSample;
  FilterInput = FilterInputSample;

  // Filter

  if (ChipModel == 8580) {
    Cutoff = CutoffMul8580_44100Hz[Cutoff];
    Resonance = Resonances8580[ResonancePos];
 }
 else { //6581
  Cutoff += (FilterInput*105)>>16; if (Cutoff>0x7FF) Cutoff=0x7FF; else if (Cutoff<0) Cutoff=0; //MOSFET-VCR control-voltage calculation
  Cutoff = CutoffMul6581_44100Hz[Cutoff]; //(resistance-modulation aka 6581 filter distortion) emulation
  Resonance = Resonances6581[ResonancePos];
 }

 FilterOutput=0;
 Tmp = FilterInput + ((PrevBandPass * Resonance)>>12) + PrevLowPass;
 if (VolumeBand & HIGHPASS_BITVAL) FilterOutput -= Tmp;
 Tmp = PrevBandPass - ( (Tmp * Cutoff) >> 12 );
 PrevBandPass = Tmp;
 if (VolumeBand & BANDPASS_BITVAL) FilterOutput -= Tmp;
 Tmp = PrevLowPass + ( (Tmp * Cutoff) >> 12 );
 PrevLowPass = Tmp;
 if (VolumeBand & LOWPASS_BITVAL) FilterOutput += Tmp;

 //Output-mixing stage

 //For $D418 volume-register digi playback: an AC / DC separation for $D418 value at low (20Hz or so) cutoff-frequency,
 //sending AC (highpass) value to a 4th 'digi' channel mixed to the master output, and set ONLY the DC (lowpass) value to the volume-control.
 //This solved 2 issues: Thanks to the lowpass filtering of the volume-control, SID tunes where digi is played together with normal SID channels,
 //won't sound distorted anymore, and the volume-clicks disappear when setting SID-volume. (This is useful for fade-in/out tunes like Hades Nebula, where clicking ruins the intro.)
 if (RealSIDmode) {
  Tmp = (signed int) ( (VolumeBand&0xF) << 12 );
  NonFilted += (Tmp - PrevVolume) * D418_DIGI_VOLUME; //highpass is digi, adding it to output must be before digifilter-code
  PrevVolume += (Tmp - PrevVolume) >> 10; //arithmetic shift amount determines digi lowpass-frequency
  MainVolume = PrevVolume >> 12; //lowpass is main volume
 }
 else MainVolume = VolumeBand & 0xF;

 Output = ((NonFilted+FilterOutput) * MainVolume) / ( (CHANNELS*VOLUME_MAX) + ATTENUATION);
 return Output; // master output of a SID
}



cRSID::cRSID(unsigned short model, bool realsid, unsigned short samplerate):ChipModel(model), RealSIDmode(realsid) {
  SampleClockRatio = (C64_PAL_CPUCLK << 4) / samplerate;
  static unsigned char Channel;
  for (Channel = 0; Channel < 21; Channel += 7) {
    ADSRstate[Channel] = 0;
    RateCounter[Channel] = 0;
    EnvelopeCounter[Channel] = 0;
    ExponentCounter[Channel] = 0;
    PhaseAccu[Channel] = 0;
    PrevPhaseAccu[Channel] = 0;
    NoiseLFSR[Channel] = 0x7FFFFF;
    PrevWavGenOut[Channel] = 0;
    PrevWavData[Channel] = 0;
  }
  SyncSourceMSBrise = 0;
  RingSourceMSB = 0;
  PrevLowPass = PrevBandPass = PrevVolume = 0;
}

void cRSID::cRSID_emulateADSRs (char cycles) {

 enum ADSRstateBits { GATE_BITVAL=0x01, ATTACK_BITVAL=0x80, DECAYSUSTAIN_BITVAL=0x40, HOLDZEROn_BITVAL=0x10 };

 static const short ADSRprescalePeriods[16] = {
  9, 32, 63, 95, 149, 220, 267, 313, 392, 977, 1954, 3126, 3907, 11720, 19532, 31251
 };
 static const unsigned char ADSRexponentPeriods[256] = {
  1, 30, 30, 30, 30, 30, 30, 16, 16, 16, 16, 16, 16, 16, 16,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, //pos0:1  pos6:30  pos14:16  pos26:8
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, //pos54:4 //pos93:2
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
 };

 static unsigned char Channel, PrevGate, AD, SR;
 static unsigned short PrescalePeriod;
 static unsigned char *ADSRstatePtr, *EnvelopeCounterPtr, *ExponentCounterPtr;
 static unsigned short *RateCounterPtr;


 for (Channel=0; Channel<21; Channel+=7) {

  AD=Register[Channel + 5];
  SR=Register[Channel + 6];
  ADSRstatePtr = &(ADSRstate[Channel]);
  RateCounterPtr = &(RateCounter[Channel]);
  EnvelopeCounterPtr = &(EnvelopeCounter[Channel]);
  ExponentCounterPtr = &(ExponentCounter[Channel]);

  PrevGate = (*ADSRstatePtr & GATE_BITVAL);
  if ( PrevGate != (Register[Channel + 4] & GATE_BITVAL) ) { //gatebit-change?
   if (PrevGate) *ADSRstatePtr &= ~ (GATE_BITVAL | ATTACK_BITVAL | DECAYSUSTAIN_BITVAL); //falling edge
   else *ADSRstatePtr = (GATE_BITVAL | ATTACK_BITVAL | DECAYSUSTAIN_BITVAL | HOLDZEROn_BITVAL); //rising edge
  }

  if (*ADSRstatePtr & ATTACK_BITVAL) PrescalePeriod = ADSRprescalePeriods[ AD >> 4 ];
  else if (*ADSRstatePtr & DECAYSUSTAIN_BITVAL) PrescalePeriod = ADSRprescalePeriods[ AD & 0x0F ];
  else PrescalePeriod = ADSRprescalePeriods[ SR & 0x0F ];

  *RateCounterPtr += cycles; if (*RateCounterPtr >= 0x8000) *RateCounterPtr -= 0x8000; //*RateCounterPtr &= 0x7FFF; //can wrap around (ADSR delay-bug: short 1st frame)

  if (PrescalePeriod <= *RateCounterPtr && *RateCounterPtr < PrescalePeriod+cycles) { //ratecounter shot (matches rateperiod) (in genuine SID ratecounter is LFSR)
   *RateCounterPtr -= PrescalePeriod; //reset rate-counter on period-match
   if ( (*ADSRstatePtr & ATTACK_BITVAL) || ++(*ExponentCounterPtr) == ADSRexponentPeriods[*EnvelopeCounterPtr] ) {
    *ExponentCounterPtr = 0;
    if (*ADSRstatePtr & HOLDZEROn_BITVAL) {
     if (*ADSRstatePtr & ATTACK_BITVAL) {
      ++(*EnvelopeCounterPtr);
      if (*EnvelopeCounterPtr==0xFF) *ADSRstatePtr &= ~ATTACK_BITVAL;
     }
     else if ( !(*ADSRstatePtr & DECAYSUSTAIN_BITVAL) || *EnvelopeCounterPtr != (SR&0xF0)+(SR>>4) ) {
      --(*EnvelopeCounterPtr); //resid adds 1 cycle delay, we omit that mechanism here
      if (*EnvelopeCounterPtr==0) *ADSRstatePtr &= ~HOLDZEROn_BITVAL;
     }
    }
   }
  }
 }
}

unsigned short cRSID::combinedWF(const unsigned char *WFarray,
                                 unsigned short oscval, unsigned char Channel) {
  static unsigned char Pitch;
  static unsigned short Filt;
  if (ChipModel == 6581 && WFarray != PulseTriangle)
    oscval &= 0x7FFF;
  Pitch = Register[Channel + 1] ? Register[Channel + 1]
                                : 1; // avoid division by zero
  Filt = 0x7777 + (0x8888 / Pitch);
  PrevWavData[Channel] =
      (WFarray[oscval >> 4] * Filt + PrevWavData[Channel] * (0xFFFF - Filt)) >>
      16;
  return PrevWavData[Channel] << 8;
}

int cRSID::cRSID_emulateWaves () {
 enum WaveFormBits { NOISE_BITVAL=0x80, PULSE_BITVAL=0x40, SAW_BITVAL=0x20, TRI_BITVAL=0x10 };
 enum ControlBits { TEST_BITVAL=0x08, RING_BITVAL=0x04, SYNC_BITVAL=0x02, GATE_BITVAL=0x01 };
 enum FilterBits { OFF3_BITVAL=0x80 };


 static const unsigned char FilterSwitchVal[] = {1,1,1,1,1,1,1,2,2,2,2,2,2,2,4};

 static char MainVolume;
 static unsigned char Channel, WF, TestBit, Envelope, FilterSwitchReso, VolumeBand;
 static unsigned int Utmp, PhaseAccuStep, MSB, WavGenOut, PW;
 static int Tmp, Feedback, Steepness, PulsePeak;
 //static int FilterInput, Cutoff, Resonance, FilterOutput, NonFilted, Output;
 static int *PhaseAccuPtr;



 NonFiltedSample = FilterInputSample = 0;
 FilterSwitchReso = Register[0x17]; VolumeBand=Register[0x18];


 //Waveform-generator //(phase accumulator and waveform-selector)


 for (Channel=0; Channel<21; Channel+=7) {

  WF = Register[Channel + 4]; TestBit = ( (WF & TEST_BITVAL) != 0 );
  PhaseAccuPtr = &(PhaseAccu[Channel]);

  PhaseAccuStep = ( (Register[Channel + 1]<<8) + Register[Channel + 0] ) * SampleClockRatio;
  if (TestBit || ((WF & SYNC_BITVAL) && SyncSourceMSBrise)) *PhaseAccuPtr = 0;
  else { //stepping phase-accumulator (oscillator)
   *PhaseAccuPtr += PhaseAccuStep;
   if (*PhaseAccuPtr >= 0x10000000) *PhaseAccuPtr -= 0x10000000;
  }
  *PhaseAccuPtr &= 0xFFFFFFF;
  MSB = *PhaseAccuPtr & 0x8000000;
  SyncSourceMSBrise = (MSB > (PrevPhaseAccu[Channel] & 0x8000000)) ? 1 : 0;


  if (WF & NOISE_BITVAL) { //noise waveform
   Tmp = NoiseLFSR[Channel]; //clock LFSR all time if clockrate exceeds observable at given samplerate (last term):
   if ( ((*PhaseAccuPtr & 0x1000000) != (PrevPhaseAccu[Channel] & 0x1000000)) || PhaseAccuStep >= 0x1000000 ) {
    Feedback = ( (Tmp & 0x400000) ^ ((Tmp & 0x20000) << 5) ) != 0;
    Tmp = ( (Tmp << 1) | Feedback|TestBit ) & 0x7FFFFF; //TEST-bit turns all bits in noise LFSR to 1 (on real SID slowly, in approx. 8000 microseconds ~ 300 samples)
    NoiseLFSR[Channel] = Tmp;
   } //we simply zero output when other waveform is mixed with noise. On real SID LFSR continuously gets filled by zero and locks up. ($C1 waveform with pw<8 can keep it for a while.)
   WavGenOut = (WF & 0x70) ? 0 : ((Tmp & 0x100000) >> 5) | ((Tmp & 0x40000) >> 4) | ((Tmp & 0x4000) >> 1) | ((Tmp & 0x800) << 1)
                                 | ((Tmp & 0x200) << 2) | ((Tmp & 0x20) << 5) | ((Tmp & 0x04) << 7) | ((Tmp & 0x01) << 8);
  }

  else if (WF & PULSE_BITVAL) { //simple pulse
   PW = ( ((Register[Channel + 3]&0xF) << 8) + Register[Channel + 2] ) << 4; //PW=0000..FFF0 from SID-register
   Utmp = (int) (PhaseAccuStep >> 13); if (0 < PW && PW < Utmp) PW = Utmp; //Too thin pulsewidth? Correct...
   Utmp ^= 0xFFFF;  if (PW > Utmp) PW = Utmp; //Too thin pulsewidth? Correct it to a value representable at the current samplerate
   Utmp = *PhaseAccuPtr >> 12;

   if ( (WF&0xF0) == PULSE_BITVAL ) { //simple pulse, most often used waveform, make it sound as clean as possible (by making it trapezoid)
    Steepness = (PhaseAccuStep>=4096) ? 0xFFFFFFF/PhaseAccuStep : 0xFFFF; //rising/falling-edge steepness (add/sub at samples)
    if (TestBit) WavGenOut = 0xFFFF;
    else if (Utmp<PW) { //rising edge (interpolation)
     PulsePeak = (0xFFFF-PW) * Steepness; //very thin pulses don't make a full swing between 0 and max but make a little spike
     if (PulsePeak>0xFFFF) PulsePeak=0xFFFF; //but adequately thick trapezoid pulses reach the maximum level
     Tmp = PulsePeak - (PW-Utmp)*Steepness; //draw the slope from the peak
     WavGenOut = (Tmp<0)? 0:Tmp;           //but stop at 0-level
    }
    else { //falling edge (interpolation)
     PulsePeak = PW*Steepness; //very thin pulses don't make a full swing between 0 and max but make a little spike
     if (PulsePeak>0xFFFF) PulsePeak=0xFFFF; //adequately thick trapezoid pulses reach the maximum level
     Tmp = (0xFFFF-Utmp)*Steepness - PulsePeak; //draw the slope from the peak
     WavGenOut = (Tmp>=0)? 0xFFFF:Tmp;         //but stop at max-level
   }}

   else { //combined pulse
    WavGenOut = (Utmp >= PW || TestBit) ? 0xFFFF:0;
    if (WF & TRI_BITVAL) {
     if (WF & SAW_BITVAL) { //pulse+saw+triangle (waveform nearly identical to tri+saw)
       if (WavGenOut) WavGenOut = combinedWF( PulseSawTriangle, Utmp, Channel);
     }
     else { //pulse+triangle
      Tmp = *PhaseAccuPtr ^ ( (WF&RING_BITVAL)? RingSourceMSB : 0 );
      if (WavGenOut) WavGenOut = combinedWF( PulseTriangle, Tmp >> 12, Channel);
    }}
    else if (WF & SAW_BITVAL) { //pulse+saw
      if(WavGenOut) WavGenOut = combinedWF( PulseSawtooth, Utmp, Channel);
   }}
  }

  else if (WF & SAW_BITVAL) { //sawtooth
   WavGenOut = *PhaseAccuPtr >> 12; //saw (this row would be enough for simple but aliased-at-high-pitch saw)
   if (WF & TRI_BITVAL) WavGenOut = combinedWF( SawTriangle, WavGenOut, Channel); //saw+triangle
   else { //simple cleaned (bandlimited) saw
    Steepness = (PhaseAccuStep>>4)/288; if(Steepness==0) Steepness=1; //avoid division by zero in next steps
    WavGenOut += (WavGenOut * Steepness) >> 16; //1st half (rising edge) of asymmetric triangle-like saw waveform
    if (WavGenOut>0xFFFF) WavGenOut = 0xFFFF - ( ((WavGenOut-0x10000)<<16) / Steepness ); //2nd half (falling edge, reciprocal steepness)
  }}

  else if (WF & TRI_BITVAL) { //triangle (this waveform has no harsh edges, so it doesn't suffer from strong aliasing at high pitches)
   Tmp = *PhaseAccuPtr ^ ( WF&RING_BITVAL? RingSourceMSB : 0 );
   WavGenOut = ( Tmp ^ (Tmp&0x8000000? 0xFFFFFFF:0) ) >> 11;
  }


  WavGenOut &= 0xFFFF;
  if (WF&0xF0) PrevWavGenOut[Channel] = WavGenOut; //emulate waveform 00 floating wave-DAC (utilized by SounDemon digis)
  else WavGenOut = PrevWavGenOut[Channel];  //(on real SID waveform00 decays, we just simply keep the value to avoid clicks)
  PrevPhaseAccu[Channel] = *PhaseAccuPtr;
  RingSourceMSB = MSB;

  //routing the channel signal to either the filter or the unfiltered master output depending on filter-switch SID-registers
  Envelope = ChipModel==8580 ?  EnvelopeCounter[Channel] : ADSR_DAC_6581[EnvelopeCounter[Channel]];
  if (FilterSwitchReso & FilterSwitchVal[Channel]) {
   FilterInputSample += ( ((int)WavGenOut-0x8000) * Envelope ) >> 8;
  }
  else if ( Channel!=14 || !(VolumeBand & OFF3_BITVAL) ) {
   NonFiltedSample += ( ((int)WavGenOut-0x8000) * Envelope ) >> 8;
  }
 }
 //update readable SID1-registers (some SID tunes might use 3rd channel ENV3/OSC3 value as control)
 // SID->C64->IObankRD[SID->BaseAddress+0x1B] = WavGenOut>>8; //OSC3, ENV3 (some players rely on it, unfortunately even for timing)
 // SID->C64->IObankRD[SID->BaseAddress+0x1C] = SID->EnvelopeCounter[14]; //Envelope

 return cRSID_emulateSIDoutputStage();
}



//----------------------- High-quality (oversampled) waveform-generation --------------------------

unsigned short cRSID::HQcombinedWF(const unsigned char *WFarray,
                                   unsigned short oscval, unsigned char Channel) {
  if (ChipModel == 6581 && WFarray != PulseTriangle)
    oscval &= 0x7FFF;
  return WFarray[oscval >> 4] << 8;
}

cRSID_SIDwavOutput cRSID::cRSID_emulateHQwaves (char cycles) {
 enum WaveFormBits { NOISE_BITVAL=0x80, PULSE_BITVAL=0x40, SAW_BITVAL=0x20, TRI_BITVAL=0x10 };
 enum ControlBits { TEST_BITVAL=0x08, RING_BITVAL=0x04, SYNC_BITVAL=0x02, GATE_BITVAL=0x01 };
 enum FilterBits { OFF3_BITVAL=0x80 };


 static unsigned char Channel, WF, TestBit, Envelope, FilterSwitchReso, VolumeBand;
 static unsigned int Utmp, PhaseAccuStep, MSB, WavGenOut, PW;
 static int Tmp, Feedback;
 //static int FilterInput, Cutoff, Resonance; //, FilterOutput, NonFilted, Output;
 static int *PhaseAccuPtr;
 static cRSID_SIDwavOutput SIDwavOutput;

 static const unsigned char FilterSwitchVal[] = {1,1,1,1,1,1,1,2,2,2,2,2,2,2,4};


 SIDwavOutput.FilterInput = SIDwavOutput.NonFilted = 0;
 FilterSwitchReso = Register[0x17]; VolumeBand=Register[0x18];

 for (Channel=0; Channel<21; Channel+=7) {

  WF = Register[Channel + 4]; TestBit = ( (WF & TEST_BITVAL) != 0 );
  PhaseAccuPtr = &(PhaseAccu[Channel]);

  PhaseAccuStep = ( (Register[Channel + 1]<<8) + Register[Channel + 0] ) * cycles;
  if (TestBit || ((WF & SYNC_BITVAL) && SyncSourceMSBrise)) *PhaseAccuPtr = 0;
  else { //stepping phase-accumulator (oscillator)
   *PhaseAccuPtr += PhaseAccuStep;
   if (*PhaseAccuPtr >= 0x1000000) *PhaseAccuPtr -= 0x1000000;
  }
  *PhaseAccuPtr &= 0xFFFFFF;
  MSB = *PhaseAccuPtr & 0x800000;
  SyncSourceMSBrise = (MSB > (PrevPhaseAccu[Channel] & 0x800000)) ? 1 : 0;


  if (WF & NOISE_BITVAL) { //noise waveform
   Tmp = NoiseLFSR[Channel]; //clock LFSR all time if clockrate exceeds observable at given samplerate (last term):

   if ( ((*PhaseAccuPtr & 0x100000) != (PrevPhaseAccu[Channel] & 0x100000)) ) {
    Feedback = ( (Tmp & 0x400000) ^ ((Tmp & 0x20000) << 5) ) != 0;
    Tmp = ( (Tmp << 1) | Feedback|TestBit ) & 0x7FFFFF; //TEST-bit turns all bits in noise LFSR to 1 (on real SID slowly, in approx. 8000 microseconds ~ 300 samples)
    NoiseLFSR[Channel] = Tmp;
   } //we simply zero output when other waveform is mixed with noise. On real SID LFSR continuously gets filled by zero and locks up. ($C1 waveform with pw<8 can keep it for a while.)
   WavGenOut = (WF & 0x70) ? 0 : ((Tmp & 0x100000) >> 5) | ((Tmp & 0x40000) >> 4) | ((Tmp & 0x4000) >> 1) | ((Tmp & 0x800) << 1)
                                 | ((Tmp & 0x200) << 2) | ((Tmp & 0x20) << 5) | ((Tmp & 0x04) << 7) | ((Tmp & 0x01) << 8);
  }
  else if (WF & PULSE_BITVAL) { //simple pulse or pulse+combined
   PW = ( ((Register[Channel + 3]&0xF) << 8) + Register[Channel + 2] ) << 4; //PW=0000..FFF0 from SID-register
   Utmp = *PhaseAccuPtr >> 8;
   WavGenOut = (Utmp >= PW || TestBit) ? 0xFFFF:0;
   if ( (WF&0xF0) != PULSE_BITVAL ) { //combined pulse
    if (WF & TRI_BITVAL) {
     if (WF & SAW_BITVAL) { //pulse+saw+triangle (waveform nearly identical to tri+saw)
       if (WavGenOut) WavGenOut = HQcombinedWF( PulseSawTriangle, Utmp, Channel);
     }
     else { //pulse+triangle
      Tmp = *PhaseAccuPtr ^ ( (WF&RING_BITVAL)? RingSourceMSB : 0 );
      if (WavGenOut) WavGenOut = HQcombinedWF( PulseTriangle, Tmp >> 8, Channel);
    }}
    else if (WF & SAW_BITVAL) { //pulse+saw
      if(WavGenOut) WavGenOut = HQcombinedWF( PulseSawtooth, Utmp, Channel);
   }}
  }
  else if (WF & SAW_BITVAL) { //sawtooth
   WavGenOut = *PhaseAccuPtr >> 8;
   if (WF & TRI_BITVAL) WavGenOut = HQcombinedWF( SawTriangle, WavGenOut, Channel); //saw+triangle
  }
  else if (WF & TRI_BITVAL) { //triangle (this waveform has no harsh edges, so it doesn't suffer from strong aliasing at high pitches)
   Tmp = *PhaseAccuPtr ^ ( WF&RING_BITVAL? RingSourceMSB : 0 );
   WavGenOut = ( Tmp ^ (Tmp&0x800000? 0xFFFFFF:0) ) >> 7;
  }

  WavGenOut &= 0xFFFF;
  if (WF&0xF0) PrevWavGenOut[Channel] = WavGenOut; //emulate waveform 00 floating wave-DAC (utilized by SounDemon digis)
  else WavGenOut = PrevWavGenOut[Channel];  //(on real SID waveform00 decays, we just simply keep the value to avoid clicks)
  PrevPhaseAccu[Channel] = *PhaseAccuPtr;
  RingSourceMSB = MSB;

  //routing the channel signal to either the filter or the unfiltered master output depending on filter-switch SID-registers
  Envelope = ChipModel==8580 ? EnvelopeCounter[Channel] : ADSR_DAC_6581[EnvelopeCounter[Channel]];
  if (FilterSwitchReso & FilterSwitchVal[Channel]) {
   SIDwavOutput.FilterInput += ( ((int)WavGenOut-0x8000) * Envelope ) >> 8;
  }
  else if ( Channel!=14 || !(VolumeBand & OFF3_BITVAL) ) {
   SIDwavOutput.NonFilted += ( ((int)WavGenOut-0x8000) * Envelope ) >> 8;
  }

 }
 //update readable SID1-registers (some SID tunes might use 3rd channel ENV3/OSC3 value as control)
 // SID->C64->IObankRD[SID->BaseAddress+0x1B] = WavGenOut>>8; //OSC3, ENV3 (some players rely on it, unfortunately even for timing)
 // SID->C64->IObankRD[SID->BaseAddress+0x1C] = EnvelopeCounter[14]; //Envelope

 //SIDwavOutput.NonFilted=NonFilted; SIDwavOutput.FilterInput=FilterInput; //SID->FilterInputCycle=FilterInput; //SID->NonFiltedCycle=NonFilted;
 return SIDwavOutput; //NonFilted; //+FilterInput; //WavGenOut; //(*PhaseAccuPtr)>>8;
}
