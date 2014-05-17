#ifndef _UI_INT_FIELD_H_
#define _UI_INT_FIELD_H_

#include "UIField.h"

class UIIntField: public UIField {

public:

	UIIntField(GUIPoint &position,int *src,const char *format,int min,int max,int xOffset,int yOffset) ;
	virtual ~UIIntField() {} ;
	virtual void Draw(GUIWindow &w) ;
	virtual void ProcessArrow(unsigned short mask) ;
	virtual void OnClick() {} ;
protected:

protected:
	int *src_ ;
	const char *format_ ;
	int min_ ;
	int max_ ;
	int xOffset_ ;
	int yOffset_ ;

} ;

#endif
