
#include "UIIntField.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "System/Console/Trace.h"
#include "Application/AppWindow.h"

#define abs(x) (x<0?-x:x)

UIIntField::UIIntField(GUIPoint &position,int *src,const char *format,int min,int max,int xOffset,int yOffset):UIField(position) {
	src_=src ;
	format_=format ;
	min_=min ;
	max_=max ;
	xOffset_=xOffset ;
	yOffset_=yOffset ;
} ;

void UIIntField::Draw(GUIWindow &w) {

	GUITextProperties props ;
	GUIPoint position=GetPosition() ;
	
	if (focus_) {
		((AppWindow&)w).SetColor(CD_HILITE2) ;
		props.invert_=true ;
	} else {
		((AppWindow&)w).SetColor(CD_NORMAL) ;
	}

	char buffer[80] ;
	int value=*src_ ;
	sprintf(buffer,format_,value) ;
	w.DrawString(buffer,position,props) ;
	
	
} ;

void UIIntField::ProcessArrow(unsigned short mask){

	int value=*src_ ;
	
	switch(mask) {
		case EPBM_UP:
			value+=yOffset_ ;
			break ;
		case EPBM_DOWN:
			value-=yOffset_ ;
			break ;
		case EPBM_LEFT:
			value-=xOffset_ ;
			break ;
  		case EPBM_RIGHT:
			value+=xOffset_ ;
			break ;
	} ;
	if (value<min_) {
		value=min_ ;
	} ;
	if (value>max_) {
		value=max_ ;
	}

	*src_=value ;
} ;
