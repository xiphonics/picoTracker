
#include "GUIEvent.h"
#include "System/Console/Trace.h"

// Typed constructor

GUIEvent::GUIEvent(GUIPoint &point, GUIEventType type, long when, bool ctrl,
                   bool shift, bool btn)
    : _position(point), _type(type), _value(0), _when(when), _ctrl(ctrl),
      _shift(shift), _btn(btn){};

GUIEvent::GUIEvent(long value, GUIEventType type, long when, bool ctrl,
                   bool shift, bool btn)
    : _position(), _type(type), _value(value), _when(when), _ctrl(ctrl),
      _shift(shift), _btn(btn){};

// Position accessor

void GUIEvent::SetPosition(GUIPoint &point) { _position = point; }

// Position accessor

GUIPoint GUIEvent::GetPosition() { return _position; }

// Type accessor

GUIEventType GUIEvent::GetType() { return _type; }
