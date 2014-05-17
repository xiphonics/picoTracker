#ifndef _UI_SORTED_VAR_LIST_H_
#define _UI_SORTED_VAR_LIST_H_

#include "UIIntVarField.h"

class UISortedVarList: public UIIntVarField {

public:

	UISortedVarList(GUIPoint &position,Variable &v,const char *format) ;
	virtual ~UISortedVarList() {} ;
	virtual void ProcessArrow(unsigned short mask) ;
} ;

#endif
