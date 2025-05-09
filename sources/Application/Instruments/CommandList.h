
#ifndef _COMMAND_LIST_H_
#define _COMMAND_LIST_H_

#include "Foundation/Types/Types.h"

class CommandList {
public:
  static FourCC GetNext(FourCC current);
  static FourCC GetPrev(FourCC current);
  static FourCC GetNextAlpha(FourCC current);
  static FourCC GetPrevAlpha(FourCC current);

  // Applies command-specific range limits to parameter values
  // Currently handles:
  // - VEL: Ensures MIDI velocity values don't exceed 127 (0x7F)
  // Can be extended to handle other commands in the future
  // Returns the range-limited parameter value
  static ushort RangeLimitCommandParam(FourCC command, ushort paramValue);
};
#endif
