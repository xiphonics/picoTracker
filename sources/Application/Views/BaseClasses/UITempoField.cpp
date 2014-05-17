#include "UITempoField.h"
#include "Application/Player/Player.h"
#include "Application/Commands/ApplicationCommandDispatcher.h"
#include "System/System/System.h"

UITempoField::UITempoField(FourCC action,GUIPoint &position,Variable &v,const char *format,int min,int max,int xOffset,int yOffset):UIIntVarField(position,v,format,min,max,xOffset,yOffset) {
	action_=action ;
	((WatchedVariable &)v).AddObserver(*this) ;
} ;

/*void UITempoField::OffsetValue(int offset) {
	int oldVal=*src_ ;
	UIIntField::OffsetValue(offset) ;
	if (*src_!=oldVal) {
		Player *player=Player::GetInstance() ;
		player->SetTempo(*src_) ;
	} ;
} ;
*/

void UITempoField::OnBClick() {
	ApplicationCommandDispatcher::GetInstance()->OnTempoTap() ;
} ;

void UITempoField::Update(Observable &,I_ObservableData *data) {
	SetChanged() ;
	NotifyObservers((I_ObservableData *)action_) ;
}

void UITempoField::ProcessArrow(unsigned short mask) {
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

void UITempoField::ProcessBArrow(unsigned short mask) {

	ApplicationCommandDispatcher *dispatcher=ApplicationCommandDispatcher::GetInstance() ;
	switch(mask) {
		case EPBM_UP:
			break ;
		case EPBM_DOWN:
			break ;
		case EPBM_LEFT:
			dispatcher->OnNudgeDown() ;
			break ;
  		case EPBM_RIGHT:
			dispatcher->OnNudgeUp() ;
			break ;
	} ;
} ;
