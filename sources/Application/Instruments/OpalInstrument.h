#ifndef _OPAL_INSTRUMENT_H_
#define _OPAL_INSTRUMENT_H_

#include "Application/Model/Song.h"
#include "Externals/opal/opal.h"
#include "I_Instrument.h"

#define OIP_MODE MAKE_FOURCC('O', 'M', 'D', 'E')      // 2-op/4-op
#define OIP_FEEDBACK MAKE_FOURCC('O', 'F', 'B', 'C')  // feedback
#define OIP_ALGO MAKE_FOURCC('O', 'A', 'L', 'G')      // Algo
#define OIP_ADSR1 MAKE_FOURCC('O', '1', 'A', 'D')     // ADSR
#define OIP_WAVEFORM1 MAKE_FOURCC('O', '1', 'W', 'F') // Waveform
#define OIP_OUTPUT1 MAKE_FOURCC('O', '1', 'L', 'V')   // Output level
#define OIP_SCALE1 MAKE_FOURCC('O', '1', 'K', 'S')    // Key scale
#define OIP_ADSR2 MAKE_FOURCC('O', '2', 'A', 'D')     // ADSR
#define OIP_WAVEFORM2 MAKE_FOURCC('O', '2', 'W', 'F') // Waveform
#define OIP_OUTPUT2 MAKE_FOURCC('O', '2', 'L', 'V')   // Output level
#define OIP_SCALE2 MAKE_FOURCC('O', '2', 'K', 'S')    // Key scale
#define OIP_ADSR3 MAKE_FOURCC('O', '3', 'A', 'D')     // ADSR
#define OIP_WAVEFORM3 MAKE_FOURCC('O', '3', 'W', 'F') // Waveform
#define OIP_OUTPUT3 MAKE_FOURCC('O', '3', 'L', 'V')   // Output level
#define OIP_SCALE3 MAKE_FOURCC('O', '3', 'K', 'S')    // Key scale
#define OIP_ADSR4 MAKE_FOURCC('O', '4', 'A', 'D')     // ADSR
#define OIP_WAVEFORM4 MAKE_FOURCC('O', '4', 'W', 'F') // Waveform
#define OIP_OUTPUT4 MAKE_FOURCC('O', '4', 'L', 'V')   // Output level
#define OIP_SCALE4 MAKE_FOURCC('O', '4', 'K', 'S')    // Key scale

class OpalInstrument : public I_Instrument {

public:
  OpalInstrument();
  virtual ~OpalInstrument();

  virtual bool Init();

  // Start & stop the instument
  virtual bool Start(int channel, unsigned char note, bool retrigger = true);
  virtual void Stop(int channel);

  // size refers to the number of samples
  // should always fill interleaved stereo / 16bit
  virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
  virtual void ProcessCommand(int channel, FourCC cc, ushort value);

  virtual bool IsInitialized();

  virtual bool IsEmpty() { return false; };

  virtual InstrumentType GetType() { return IT_OPAL; };

  virtual etl::string<24> GetName();

  virtual void OnStart();

  virtual void Purge(){};

  virtual int GetTable();
  virtual bool GetTableAutomation();
  virtual void GetTableState(TableSaveState &state);
  virtual void SetTableState(TableSaveState &state);

private:
  Opal opl_;
};

#endif
