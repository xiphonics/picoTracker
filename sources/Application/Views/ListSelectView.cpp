
#include "ListSelectView.h"
#include "BaseClasses/UIActionField.h"

#define LIST_SELECT 1
#define LIST_SIZE 21

ListSelectView::ListSelectView(GUIWindow &w,ViewData *viewData):FieldView(w,viewData) {
}

void ListSelectView::OnFocus() {
} ;

void ListSelectView::OnPlayerUpdate(PlayerEventType eventType,unsigned int tick) {
}


void ListSelectView::ProcessButtonMask(unsigned short mask,bool pressed) {
	FieldView::ProcessButtonMask(mask) ;
}

void ListSelectView::SetContent(T_SimpleList<Path> &content)  {

	T_SimpleList<UIField>::Empty() ;
	GUIPoint position=GetAnchor() ;
	IteratorPtr<Path> it(content.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()){
		Path &current=it->CurrentItem() ;
		UIActionField *a1=new UIActionField(current.GetPath(),LIST_SELECT,position) ;
		a1->AddObserver(*this) ;
		T_SimpleList<UIField>::Insert(a1) ;	
		position._y+=1 ;
	}
	topIndex_=0 ;
} ;

void ListSelectView::Update(Observable &o,I_ObservableData *) {
	UIActionField &f=(UIActionField	&)o ;
	ViewEvent ve(VET_LIST_SELECT,(void *)(f.GetString())) ;
	SetChanged();
	NotifyObservers(&ve) ;
}

void ListSelectView::DrawView() {
   
	Clear() ;

	GUITextProperties props ;
	GUIPoint pos=GetTitlePosition() ;

// Draw title

	SetColor(CD_NORMAL) ;
	DrawString(pos._x,pos._y,"Press A to select project",props) ;


 // Set focus if not existing

	UIField *focus=GetFocus() ;
	if (focus==0) {
		focus=T_SimpleList<UIField>::GetFirst() ;
		SetFocus(focus) ;
	}

	// Get focus Index

	int focusIndex=GetFocusIndex() ;

	// Check if we're out of bound

	if (focusIndex<topIndex_) {
		topIndex_=focusIndex ;
	} ;
	if (focusIndex>=topIndex_+LIST_SIZE) {
		topIndex_=focusIndex-LIST_SIZE+1 ;
	} ;

 // Draw items

	GUIPoint anchor=GetAnchor() ;

	IteratorPtr<UIField> it2(T_SimpleList<UIField>::GetIterator()) ;

	int count=0 ; 
	int offset=0 ;

	for (it2->Begin();!it2->IsDone();it2->Next()) {
		if (count==topIndex_) {
			UIField &current=it2->CurrentItem() ;
			offset=-current.GetPosition()._y+anchor._y ;
		}
		if (count>=topIndex_) {
			UIField &current=it2->CurrentItem() ;
			current.Draw(w_,offset) ;
		} ;
		count++ ;
		if (count>=topIndex_+LIST_SIZE) {
			break ;
		} ;
	} ;
};
