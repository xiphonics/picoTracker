#ifndef _NULL_VIEW_H_
#define _NULL_VIEW_H_

#include "BaseClasses/View.h"
#include "ViewData.h"

class NullView: public View {
public:
	NullView(GUIWindow &w,ViewData *viewData) ;
	~NullView() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int tick=0) ;
	virtual void OnFocus() ;
private:

} ;
#endif

