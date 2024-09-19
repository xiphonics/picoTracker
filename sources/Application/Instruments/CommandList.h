
#ifndef _COMMAND_LIST_H_
#define _COMMAND_LIST_H_

#include "Foundation/Types/Types.h"

class CommandList {
public:
  static FourCC GetNext(FourCC current);
  static FourCC GetPrev(FourCC current);
  static FourCC GetNextAlpha(FourCC current);
  static FourCC GetPrevAlpha(FourCC current);
};
#endif
