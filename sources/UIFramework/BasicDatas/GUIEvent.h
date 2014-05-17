
#ifndef _GUIEVENT_H_
#define _GUIEVENT_H_

#include "GUIPoint.h"

// GUIEvent: a simple class to represent user interface event across the
// system.

// Event types

enum  GUIEventType { ET_NONE, ET_MOUSEDOWN, ET_MOUSEUP, ET_MOUSEMOVE , ET_RESIZE ,ET_KEYUP, ET_KEYDOWN,ET_IDLE, ET_PADBUTTONUP,ET_PADBUTTONDOWN, ET_PLAYERUPDATE,ET_SYSQUIT};

enum  GUIEventKeyType 
{ 
	EKT_LEFT		= 37,
	EKT_RIGHT		= 39,
	EKT_UP			= 38,
	EKT_DOWN		= 40,
	EKT_PAGE_UP		= 33,
	EKT_PAGE_DOWN	= 34,
	EKT_HOME		= 36,
	EKT_END			= 35,
	EKT_BACKSPACE	=  8,
	EKT_DELETE		= 46,
	EKT_ESCAPE		= 27
};

enum GUIEventPadButtonType {
EPBT_LEFT,
EPBT_DOWN,
EPBT_RIGHT, 
EPBT_UP, 
EPBT_L, 
EPBT_B, 
EPBT_A, 
EPBT_R, 
EPBT_START, 
EPBT_SELECT 
} ;

class GUIEvent {
public:
	
	// Blank constructor

	GUIEvent() { _type=ET_NONE ; } ;

	// Constructor with data

	GUIEvent(GUIPoint &point,GUIEventType type,long when=0,bool ctrl=false,bool shift=false,bool btn=false);
	GUIEvent(long value,GUIEventType type,long when=0,bool ctrl=false,bool shift=false,bool btn=false);
	
	// Position accessor

	GUIPoint GetPosition() ;
	void SetPosition(GUIPoint &) ;

	// Type accessor

	GUIEventType GetType() ;
	long	GetValue(){return _value;}; ;
	long	When(){return _when;}; ;
	bool	IsShiftPressed(){return _shift;};
	bool	IsCtrlPressed(){return _ctrl;};
	bool	IsBtnPressed(){return _btn;};

	// Utils

	void Dump() ;

private:
	GUIPoint _position ; // The event's position
	GUIEventType _type ; // The event's type
	int			 _value; // for key events...
	long         _when ; // event timestamp
	bool		 _ctrl;
	bool		 _shift;
	bool		 _btn;
} ;

#endif
