
#ifndef _LIST_SELECT_VIEW_H
#define _LIST_SELECT_VIEW_H

#include "BaseClasses/FieldView.h"
#include "Foundation/Observable.h"

class ListSelectView: public FieldView,public I_Observer  {

public:
	ListSelectView(GUIWindow &w,ViewData *viewData) ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnFocus()  ;
	virtual void OnPlayerUpdate(PlayerEventType,unsigned int tick=0) ;

	void SetContent(T_SimpleList<Path> &) ;

	// Observer for action callback

	void Update(Observable &,I_ObservableData *) ;

private:
	T_SimpleList<Path> content_ ;
	int topIndex_ ;
} ;

#endif
