#ifndef _NEW_PROJECT_DIALOG_H_
#define _NEW_PROJECT_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include <string>

#define MAX_NAME_LENGTH 12

class NewProjectDialog:public ModalView {
public:
	NewProjectDialog(View &view) ;
	virtual ~NewProjectDialog() ;

	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int currentTick) ;
	virtual void OnFocus() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;

	std::string GetName() ;
private:
	int selected_ ;
	int lastChar_ ;
	char name_[MAX_NAME_LENGTH+1] ;
	int currentChar_ ;
} ;

#endif
