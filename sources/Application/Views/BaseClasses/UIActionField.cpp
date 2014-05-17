#include "UIActionField.h"
#include "Application/AppWindow.h"

UIActionField::UIActionField(const char *name,unsigned int fourcc,GUIPoint &position):UIField(position) {
	name_=name ;
	fourcc_=fourcc ;
} ;

UIActionField::UIActionField(std::string name,unsigned int fourcc,GUIPoint &position):UIField(position) {
	name_=name ;
	fourcc_=fourcc ;
} ;

UIActionField::~UIActionField() {

} ;
void UIActionField::Draw(GUIWindow &w, int offset) {

	GUITextProperties props ;
	GUIPoint position(x_,y_+offset) ;

	if (focus_) {
		((AppWindow&)w).SetColor(CD_HILITE2) ;
		props.invert_=true ;
	} else {
		((AppWindow&)w).SetColor(CD_NORMAL) ;
	}

	w.DrawString(name_.c_str(),position,props) ;
} ;

void UIActionField::OnClick() {
	SetChanged() ;
	NotifyObservers((I_ObservableData *)fourcc_) ;
} ;

const char *UIActionField::GetString() {
	return name_.c_str() ;
} ;
