#ifndef _UI_INT_VAR_FIELD_H_
#define _UI_INT_VAR_FIELD_H_

#include "UIField.h"
#include "Foundation/Variables/Variable.h"

class UIIntVarField: public UIField {

public:

	UIIntVarField(
    GUIPoint &position,
    Variable &v,
    const char *format,
    int min,
    int max,
    int xOffset,
    int yOffset,
    int displayOffset = 0);
  
	virtual ~UIIntVarField() {} ;
	virtual void Draw(GUIWindow &w,int offset=0) ;
	virtual void ProcessArrow(unsigned short mask) ;
	virtual void OnClick() {} ;
  
  FourCC GetVariableID() ;
	Variable &GetVariable() ;
protected:
	Variable &src_ ;
	const char *format_ ;
	int min_ ;
	int max_ ;
	int xOffset_ ;
	int yOffset_ ;
  int displayOffset_;
} ;

#endif
