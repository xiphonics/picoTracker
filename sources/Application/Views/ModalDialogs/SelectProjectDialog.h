#ifndef _SELECT_PROJECT_DIALOG_H_
#define _SELECT_PROJECT_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "System/FileSystem/FileSystem.h"
#include "System/Errors/Result.h"

class SelectProjectDialog:public ModalView {
public:
	SelectProjectDialog(View &view) ;
	~SelectProjectDialog() ;

	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int currentTick) ;
	virtual void OnFocus() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;

	Result OnNewProject(std::string &name) ;

	Path GetSelection() ;

protected:
	void warpToNextProject(int amount) ;
	void setCurrentFolder(Path &path) ;

private:
	T_SimpleList<Path> content_ ;
	int topIndex_ ;
	int currentProject_ ;
	int selected_ ;
	Path currentPath_ ;
	Path selection_ ;
	static Path lastFolder_ ;
	static int lastProject_ ;
} ;

#endif
