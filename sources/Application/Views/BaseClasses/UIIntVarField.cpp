#include "UIIntVarField.h"


#include "UIIntVarField.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "System/Console/Trace.h"
#include "Application/AppWindow.h"

#define abs(x) (x<0?-x:x)

UIIntVarField::UIIntVarField(
  GUIPoint &position,
  Variable &v,
  const char *format,
  int min,
  int max,
  int xOffset,
  int yOffset,
  int displayOffset)
:UIField(position)
,src_(v) 
{
	format_=format ;
	min_=min ;
	max_=max ;
	xOffset_=xOffset ;
	yOffset_=yOffset ;
  displayOffset_ = displayOffset;
} ;

void UIIntVarField::Draw(GUIWindow &w,int offset) {

	GUITextProperties props ;
	GUIPoint position=GetPosition() ;
	position._y+=offset ;

	if (focus_) {
		((AppWindow&)w).SetColor(CD_HILITE2) ;
		props.invert_=true ;
	} else {
		((AppWindow&)w).SetColor(CD_NORMAL) ;
	}
	Variable::Type type=src_.GetType() ;
	char buffer[80] ;
	switch (type) {
		case Variable::INT:
			{
			int ivalue=src_.GetInt()+displayOffset_ ;
			sprintf(buffer,format_,ivalue,ivalue) ;
			}
			break ;
		case Variable::CHAR_LIST:
		case Variable::BOOL:
			{
			const char *cvalue=src_.GetString() ;
			sprintf(buffer,format_,cvalue) ;
			}
			break ;

		default:
			strcpy(buffer,"++wtf++");
	}
	w.DrawString(buffer,position,props) ;
} ;

void UIIntVarField::ProcessArrow(unsigned short mask) {
	int value=src_.GetInt() ;

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
	
	src_.SetInt(value) ;
} ;

FourCC UIIntVarField::GetVariableID() {
    return src_.GetID() ;
} ;

Variable &UIIntVarField::GetVariable() {
	return src_ ;
} ;
