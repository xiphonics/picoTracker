#ifndef _UI_VAR_NOTE_FIELD_H_
#define _UI_VAR_NOTE_FIELD_H_

#include "UIIntVarField.h"

class UINoteVarField: public UIIntVarField {

public:
	UINoteVarField(GUIPoint &position,Variable &v,const char *format,int min,int max,int xOffset,int yOffset) ;
	virtual void Draw(GUIWindow &w,int offset=0) ;

} ;

#endif
