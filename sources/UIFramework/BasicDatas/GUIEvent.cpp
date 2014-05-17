
#include "GUIEvent.h"
#include "System/Console/Trace.h"

// Typed constructor

GUIEvent::GUIEvent(GUIPoint &point,GUIEventType type,long when,bool ctrl,bool shift,bool btn) :_value(0),_position(point),_type(type),_when(when),_ctrl(ctrl),_shift(shift),_btn(btn) 
{
} ;

GUIEvent::GUIEvent(long value,GUIEventType type,long when,bool ctrl,bool shift,bool btn) :_position(),_value(value),_type(type),_when(when),_ctrl(ctrl),_shift(shift),_btn(btn) 
{
} ;

// Position accessor

void GUIEvent::SetPosition(GUIPoint &point) {
	_position=point ;
}

// Position accessor

GUIPoint GUIEvent::GetPosition() {
	return _position ;
}

// Type accessor

GUIEventType GUIEvent::GetType() {
	return _type ;
}

void GUIEvent::Dump() {
	const char *type ;
	switch(_type) {
		case ET_PADBUTTONUP:
		   type="ET_PADBUTTONUP" ;
		   break ;
		case ET_PADBUTTONDOWN:
		   type="ET_PADBUTTONDOWN" ;
		   break ;
		default:
		   type="unknown" ;
		   break ;
	}
	const char *value ;
	switch(_value) {
case EPBT_LEFT:
	value="EPBT_LEFT" ;
	break;
case EPBT_DOWN:
	value="EPBT_DOWN" ;
	break;
case EPBT_RIGHT: 
	value="EPBT_RIGHT" ;
	break;
case EPBT_UP:
	value="EPBT_UP" ;
	break;
case EPBT_L:
	value="EPBT_L" ;
	break;
case EPBT_B: 
	value="EPBT_B" ;
	break;
case EPBT_A: 
	value="EPBT_A" ;
	break;
case EPBT_R:
	value="EPBT_R" ;
	break;
case EPBT_START: 
	value="EPBT_START" ;
	break;
case EPBT_SELECT: 
	value="EPBT_SELECT" ;
	break;
	}
}

