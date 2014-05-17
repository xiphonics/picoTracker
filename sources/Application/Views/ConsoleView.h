
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "BaseClasses/View.h"
#include "System/Console/Trace.h"

#define CONSOLE_WIDTH 40
#define CONSOLE_HEIGHT 25

class ConsoleView: public View,public Trace {
public:
	ConsoleView(GUIWindow &w,ViewData *viewData) ;

	// View implementation

	virtual void ProcessButtonMask(unsigned short mask,bool pressed);
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType,unsigned int) {} ;
	virtual void OnFocus() {} ;

	// Trace Implementation

	virtual void AddBuffer(char *buffer) ;

	// Additional

	bool IsDirty() ;

private:
	char lines_[CONSOLE_HEIGHT][CONSOLE_WIDTH] ;
	int currentLine_ ;
} ;

#endif
