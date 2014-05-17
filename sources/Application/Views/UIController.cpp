#include "UIController.h"

#include "Application/Player/Player.h"

UIController::UIController() {
} ;

UIController *UIController::GetInstance() {
	if (instance_==0) {
		instance_=new UIController() ;
	}
	return instance_ ;
}

void UIController::Init(Project *project,ViewData *viewData) {
	viewData_=viewData ;
	project_=project ;
}

void UIController::Reset() {
	viewData_=0 ;
	project_=0 ;
} ;

void UIController::UnMuteAll() {

	Player *player=Player::GetInstance() ;
	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
			player->SetChannelMute(i,false) ;
	} ;
} ;

void UIController::ToggleMute(int from,int to) {

	Player *player=Player::GetInstance() ;
	for (int i=from;i<to+1;i++) {
		bool muted=player->IsChannelMuted(i) ;
		player->SetChannelMute(i,!muted) ;
	};
} ;

void UIController::SwitchSoloMode(int from,int to,bool soloing) {

	Player *player=Player::GetInstance() ;
	
	// If not in solo mode, we solo current channel or selection
	
	if (soloing) {

		for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
			soloMask_[i]=player->IsChannelMuted(i) ;
			player->SetChannelMute(i,(i<from)||(i>to)) ;
		} ;
	} else {
		for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
			player->SetChannelMute(i,soloMask_[i]) ;
		} ;
	}
} ;
