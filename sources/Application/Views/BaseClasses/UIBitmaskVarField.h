#ifndef _UI_BITMASK_VAR_FIELD_H_
#define _UI_BITMASK_VAR_FIELD_H_

#include "Foundation/Observable.h"
#include "UIIntVarField.h"

// Display a UI field of boolean flags stored as an integer representing a
// bitmask, ie. each bit in the byte of the variable is a single boolean flag
class UIBitmaskVarField : public UIIntVarField {

public:
  UIBitmaskVarField(GUIPoint &position, Variable &v, const char *format,
                    int len);
  virtual ~UIBitmaskVarField(){};
  virtual void Draw(GUIWindow &w, int offset = 0);
  virtual void ProcessArrow(unsigned short mask);

private:
  unsigned int len_;
  unsigned int position_ = 0;
};
#endif
