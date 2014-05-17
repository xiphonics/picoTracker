#ifndef _UI_INT_VAR_OFF_FIELD_H_
#define _UI_INT_VAR_OFF_FIELD_H_

#include "UIIntVarField.h"

class UIIntVarOffField:public UIIntVarField {
public:
	UIIntVarOffField(GUIPoint &position,Variable &v,const char *format,int min,int max,int xOffset,int yOffset) ;
	virtual void ProcessArrow(unsigned short mask) ;
	virtual void Draw(GUIWindow &w,int offset=0) ;
} ;

#endif
