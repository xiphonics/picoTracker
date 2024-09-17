#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Application/Persistency/Persistent.h"
#include "Foundation/T_Singleton.h"
#include "Foundation/Variables/VariableContainer.h"
#include "System/Console/Trace.h"

#define VAR_LINEOUT MAKE_FOURCC('L', 'O', 'U', 'T')
// MIDI Device
#define VAR_MIDI_DEVICE MAKE_FOURCC('M', 'I', 'D', 'D')
// MIDI clock sync
#define VAR_MIDI_SYNC MAKE_FOURCC('M', 'I', 'D', 'S')

#define VAR_FG_COLOR MAKE_FOURCC('F', 'G', 'C', 'L')
#define VAR_BG_COLOR MAKE_FOURCC('B', 'G', 'C', 'L')
#define VAR_HI1_COLOR MAKE_FOURCC('H', '1', 'C', 'L')
#define VAR_HI2_COLOR MAKE_FOURCC('H', '2', 'C', 'L')
#define VAR_CONSOLE_COLOR MAKE_FOURCC('C', 'N', 'C', 'L')
#define VAR_CURSOR_COLOR MAKE_FOURCC('C', 'R', 'C', 'L')
#define VAR_INFO_COLOR MAKE_FOURCC('I', 'F', 'C', 'L')
#define VAR_WARN_COLOR MAKE_FOURCC('W', 'N', 'C', 'L')
#define VAR_ERROR_COLOR MAKE_FOURCC('E', 'R', 'C', 'L')

class Config : public T_Singleton<Config>, public VariableContainer {
public:
  Config();
  ~Config();
  int GetValue(const char *key);
  void ProcessArguments(int argc, char **argv);
  void Save();

private:
  void SaveContent(tinyxml2::XMLPrinter *printer);
  void processParams(const char *name, int value);
  void useDefaultConfig();

  Variable lineOut_;
};

#endif
