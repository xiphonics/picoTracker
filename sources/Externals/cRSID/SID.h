#ifndef SID_H
#define SID_H

#define ATTENUATION 26
#define C64_PAL_CPUCLK 985248

struct cRSID_SIDwavOutput {
  signed int NonFilted;
  signed int FilterInput;
};

class cRSID {
public:
  cRSID(unsigned short model, bool realsid, unsigned short samplerate);
  int cRSID_emulateWaves();
  cRSID_SIDwavOutput cRSID_emulateHQwaves(char cycles);

  // SID-chip data:
  unsigned short     ChipModel;     //values: 8580 / 6581
  unsigned short     BaseAddress;   //SID-baseaddress location in C64-memory (IO)
  unsigned char*     BasePtr;       //SID-baseaddress location in host's memory
  unsigned char Register[29];
  unsigned short SampleClockRatio;
  bool RealSIDmode;

  // ADSR-related:
  unsigned char ADSRstate[15];
  unsigned short     RateCounter[15];
  unsigned char      EnvelopeCounter[15];
  unsigned char      ExponentCounter[15];
  //Wave-related:
  int                PhaseAccu[15];       //28bit precision instead of 24bit
  int                PrevPhaseAccu[15];   //(integerized ClockRatio fractionals, WebSID has similar solution)
  unsigned char      SyncSourceMSBrise;
  unsigned int       RingSourceMSB;
  unsigned int       NoiseLFSR[15];
  unsigned int       PrevWavGenOut[15];
  unsigned char      PrevWavData[15];
  //Filter-related:
  int                PrevLowPass;
  int                PrevBandPass;
  //Output-stage:
  int                NonFiltedSample;
  int                FilterInputSample;
  int                PrevNonFiltedSample;
  int                PrevFilterInputSample;
  signed int         PrevVolume; //lowpass-filtered version of Volume-band register

private:
  void cRSID_emulateADSRs(char cycles);
  int cRSID_emulateSIDoutputStage();
  unsigned short combinedWF(const unsigned char *WFarray,
                            unsigned short oscval, unsigned char Channel);
  unsigned short HQcombinedWF(const unsigned char *WFarray,
                              unsigned short oscval, unsigned char Channel);
};

#endif
