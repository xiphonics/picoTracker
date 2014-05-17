
#include "UINoteVarField.h"
#include "Application/Utils/char.h"
#include "Application/AppWindow.h"

UINoteVarField::UINoteVarField(GUIPoint &position,Variable &v,const char *format,int min,int max,int xOffset,int yOffset)
               :UIIntVarField(position,v,format,min,max,xOffset,yOffset) {
} ;

void UINoteVarField::Draw(GUIWindow &w,int offset) {


	GUITextProperties props ;
	GUIPoint position=GetPosition() ;
	position._y+=offset ;

	if (focus_) {
		((AppWindow&)w).SetColor(CD_HILITE2) ;
		props.invert_=true ;
	} else {
		((AppWindow&)w).SetColor(CD_NORMAL) ;
	}

	char buffer[80] ;
	char note[5] ;
	
	unsigned char pitch=src_.GetInt() ;
	note2char(pitch,note) ;
	note[4]=0 ;
	sprintf(buffer,format_,note) ;
	w.DrawString(buffer,position,props) ;
} ;
