#ifndef _NONE_INSTRUMENT_H_
#define _NONE_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "I_Instrument.h"

class NoneInstrument : public I_Instrument {

public:
  NoneInstrument();
  virtual ~NoneInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return true; };

  virtual InstrumentType GetType() { return IT_NONE; };

  virtual etl::string<24> GetName();

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);
  etl::ilist<Variable *> *Variables() { return &variables_; };

private:
  etl::string<24> name_ = "None";
  etl::list<Variable *, 1> variables_;
};
#endif