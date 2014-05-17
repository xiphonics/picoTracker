#include "NullView.h"

NullView::NullView(GUIWindow &w,ViewData *viewData):View(w,viewData) {
}

NullView::~NullView() {
} 

void NullView::ProcessButtonMask(unsigned short mask,bool pressed) {

} ;

void NullView::DrawView() {

	Clear() ;


	GUITextProperties props;
	SetColor(CD_HILITE2) ;

	char buildString[80] ;
	sprintf(buildString,"Piggy build %s%s_%s",PROJECT_NUMBER,PROJECT_RELEASE,BUILD_COUNT) ;
	GUIPoint pos ;
	pos._y=28;
	pos._x=(40-strlen(buildString))/2 ;
	DrawString(pos._x,pos._y,buildString,props) ;

} ;

void NullView::OnPlayerUpdate(PlayerEventType ,unsigned int tick) {

} ;

void NullView::OnFocus() {
} ;
