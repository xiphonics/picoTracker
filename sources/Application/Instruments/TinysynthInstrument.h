
#ifndef _TINYSYNTH_INSTRUMENT_H_
#define _TINYSYNTH_INSTRUMENT_H_

#include "I_Instrument.h"

#include "Application/Model/Song.h"
#include "Application/Utils/fixed.h"
#include "Externals/tinysynth/tinysynth.h"
#include "Foundation/Observable.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/WatchedVariable.h"

#define TXIP_VOLUME MAKE_FOURCC('V', 'O', 'L', 'M')

#define MIP_TABLE MAKE_FOURCC('T', 'A', 'B', 'L')
#define MIP_TABLEAUTO MAKE_FOURCC('T', 'B', 'L', 'A')

#define TXIP_H1 MAKE_FOURCC('T', 'X', 'H', '1')
#define TXIP_H2 MAKE_FOURCC('T', 'X', 'H', '2')
#define TXIP_H3 MAKE_FOURCC('T', 'X', 'H', '3')
#define TXIP_H4 MAKE_FOURCC('T', 'X', 'H', '4')
#define TXIP_H5 MAKE_FOURCC('T', 'X', 'H', '5')
#define TXIP_H6 MAKE_FOURCC('T', 'X', 'H', '6')
#define TXIP_V1 MAKE_FOURCC('T', 'X', 'V', '1')
#define TXIP_V2 MAKE_FOURCC('T', 'X', 'V', '2')
#define TXIP_V3 MAKE_FOURCC('T', 'X', 'V', '3')
#define TXIP_V4 MAKE_FOURCC('T', 'X', 'V', '4')
#define TXIP_V5 MAKE_FOURCC('T', 'X', 'V', '5')
#define TXIP_V6 MAKE_FOURCC('T', 'X', 'V', '6')
#define TXIP_LO MAKE_FOURCC('T', 'X', 'L', 'O') // LFO

class TinysynthInstrument : public I_Instrument {

public:
  TinysynthInstrument();
  virtual ~TinysynthInstrument();

  // I_Instrument implementation
  virtual bool Init();
  virtual bool Start(int channel, unsigned char note, bool trigger = true);
  virtual void Stop(int channel);
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual void Purge(){};

  // Engine playback  start callback
  virtual void OnStart();

  virtual etl::string<24> GetName();

  virtual InstrumentType GetType() { return IT_TINYSYNTH; };
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);
  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

private:
  bool running_;
  TinySynth *tinysynth_;
  Variable *volume_;
  Variable *lfo_;
  Variable *harmonicadsr_[HARMONICS];
  Variable *harmonicvol_[HARMONICS];
  int remainingTicks_;
  bool playing_;
  TableSaveState tableState_;
};

#endif