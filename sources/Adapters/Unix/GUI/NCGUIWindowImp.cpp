#include "NCGUIWindowImp.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "System/System/System.h"
#include "Application/Model/Config.h"
#include "System/io/Trace.h"
#include <string.h>
#include "Application/Utils/assert.h"
#include "Application/Utils/char.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include <ncurses.h>

NCGUIWindowImp::NCGUIWindowImp(GUICreateWindowParams &p) {
	initscr();			/* Start curses mode 		*/
	raw();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */	
	refresh() ;
} ;

NCGUIWindowImp::~NCGUIWindowImp() {
	refresh() ;
	endwin() ;
}


void NCGUIWindowImp::DrawChar(const char c,GUIPoint &pos,GUITextProperties &p) {
	mvaddch(pos._y,pos._x,c);
}

void NCGUIWindowImp::DrawString(const char *string,GUIPoint &pos,GUITextProperties &p,bool overlay) {
	mvprintw(pos._y,pos._x,string) ;
} ;

void NCGUIWindowImp::DrawRect(GUIRect &r) {
	NYI ;
} ;

void NCGUIWindowImp::Clear(GUIColor &c,bool overlay) {
	NYI ;
} ;

void NCGUIWindowImp::ClearRect(GUIRect &r) {
	NYI ;
} ;

void NCGUIWindowImp::SetColor(GUIColor &c) {
	NYI ;
};

void NCGUIWindowImp::Lock() {
	NYI ;
} ;

void NCGUIWindowImp::Unlock() {
	NYI ;
} ;

void NCGUIWindowImp::Flush() {
	refresh() ;
} ;

void NCGUIWindowImp::Invalidate() {
	NYI ;
} ;

void NCGUIWindowImp::PushEvent(GUIEvent &event) {
	NYI ;
} ;

GUIRect NCGUIWindowImp::GetRect() {
	NYI ;
	return GUIRect(0,0,320,240) ;
}

void NCGUIWindowImp::ProcessExpose() {
	_window->Update() ;
}

