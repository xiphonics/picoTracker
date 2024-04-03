#ifndef _SAMPLE_INSTRUMENT_H_
#define _SAMPLE_INSTRUMENT_H_

#include "I_Instrument.h"
#include "SRPUpdaters.h"
#include "SampleRenderingParams.h"

#include "Application/Model/Song.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"
#include "SoundSource.h"

enum SampleInstrumentLoopMode {
  SILM_ONESHOT = 0,
  SILM_LOOP,
  SILM_LOOP_PINGPONG,
  SILM_OSC,
  //	SILM_OSCFINE,
  SILM_LOOPSYNC,
  SILM_SLICE,
  SILM_LAST
};

#define NO_SAMPLE (-1)
#define SIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')
#define SIP_CRUSH MAKE_FOURCC('C', 'R', 'S', 'H')
#define SIP_CRUSHVOL MAKE_FOURCC('C', 'R', 'S', 'V')
#define SIP_DOWNSMPL MAKE_FOURCC('D', 'S', 'P', 'L')
#define SIP_ROOTNOTE MAKE_FOURCC('R', 'O', 'O', 'T')
#define SIP_FINETUNE MAKE_FOURCC('F', 'N', 'T', 'N')
#define SIP_PAN MAKE_FOURCC('P', 'A', 'N', '_')
#define SIP_START MAKE_FOURCC('S', 'T', 'R', 'T')
#define SIP_END MAKE_FOURCC('E', 'N', 'D', '_')
#define SIP_LOOPMODE MAKE_FOURCC('L', 'M', 'O', 'D')
#define SIP_LOOPSTART MAKE_FOURCC('L', 'S', 'T', 'A')
#define SIP_LOOPLEN MAKE_FOURCC('L', 'L', 'E', 'N')
#define SIP_INTERPOLATION MAKE_FOURCC('I', 'N', 'T', 'P')
#define SIP_SAMPLE MAKE_FOURCC('S', 'M', 'P', 'L')
#define SIP_SLICES MAKE_FOURCC('S', 'L', 'C', 'S')
#define SIP_FILTMODE MAKE_FOURCC('F', 'I', 'M', 'O')
#define SIP_FILTMIX MAKE_FOURCC('F', 'M', 'I', 'X')
#define SIP_FILTCUTOFF MAKE_FOURCC('F', 'C', 'U', 'T')
#define SIP_FILTRESO MAKE_FOURCC('F', 'R', 'E', 'S')
#define SIP_TABLE MAKE_FOURCC('T', 'A', 'B', 'L')
#define SIP_TABLEAUTO MAKE_FOURCC('T', 'B', 'L', 'A')
#define SIP_FBTUNE MAKE_FOURCC('F', 'B', 'T', 'U')
#define SIP_FBMIX MAKE_FOURCC('F', 'B', 'M', 'X')

class SampleInstrument : public I_Instrument, I_Observer {

public:
  SampleInstrument();
  virtual ~SampleInstrument();
  // I_Instrument implementation
  virtual bool Init();
  virtual bool Start(int channel, unsigned char note, bool trigger = true);
  virtual void Stop(int channel);
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual bool IsInitialized();
  virtual bool IsEmpty();

  virtual InstrumentType GetType() { return IT_SAMPLE; };
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual void Purge();
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

  bool IsMulti();

  // Engine playback  start callback

  virtual void OnStart();

  // I_Observer
  virtual void Update(Observable &o, I_ObservableData *d);
  // Additional
  void AssignSample(int i);
  int GetSampleIndex();
  int GetVolume();
  void SetVolume(int);
  int GetSampleSize(int channel = -1);
  virtual etl::string<24> GetName(); // returns sample name until real
                                     // namer is implemented

  static void EnableDownsamplingLegacy();

protected:
  void updateInstrumentData(bool search);
  void doTickUpdate(int channel);
  void doKRateUpdate(int channel);

private:
  SoundSource *source_;
  static struct renderParams renderParams_[SONG_CHANNEL_COUNT];
  bool running_;
  bool dirty_;
  TableSaveState tableState_;

  static signed char lastMidiNote_[SONG_CHANNEL_COUNT];
  static fixed lastSample_[SONG_CHANNEL_COUNT][2];
  Variable *volume_;
  Variable *crush_;
  Variable *cutoff_;
  Variable *reso_;
  Variable *table_;
  Variable *tableAuto_;
  Variable *downsample_;
  Variable *rootNote_;
  Variable *fineTune_;
  Variable *drive_;
  WatchedVariable *start_;
  WatchedVariable *loopStart_;
  WatchedVariable *loopEnd_;
  Variable *filterMix_;
  Variable *filterMode_;
  Variable *pan_;
  Variable *loopMode_;
  Variable *slices_;
  Variable *interpolation_;

  static bool useDirtyDownsampling_;
};
#endif
