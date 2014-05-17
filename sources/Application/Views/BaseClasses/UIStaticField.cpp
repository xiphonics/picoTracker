#include "UIStaticField.h"
#include "Application/AppWindow.h"

UIStaticField::UIStaticField(GUIPoint &position,const char *string):UIField(position) {
	string_=string ;
} ;

void UIStaticField::Draw(GUIWindow &w,int offset) {

	GUITextProperties props ;
	GUIPoint position=GetPosition() ;
	position._y+=offset ;
	
	((AppWindow&)w).SetColor(CD_NORMAL) ;
	w.DrawString(string_,position,props) ;	
} ;

void UIStaticField::ProcessArrow(unsigned short mask){
} ;

bool UIStaticField::IsStatic() {
	return true ;
} ;
