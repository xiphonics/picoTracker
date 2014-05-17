
#ifndef _GUIPOINT_H_
#define _GUIPOINT_H_

// A Simple class to represent a Point/Position

class GUIPoint {
public:
	GUIPoint(long x=0,long y=0) {_x=x ; _y=y ; } ;
	void Add(GUIPoint p) { _x=_x+p._x ; _y=_y+p._y ; } ;
	void Sub(GUIPoint p) { _x=_x-p._x ; _y=_y-p._y ; } ;
  bool operator==(const GUIPoint& p) const { return _x == p._x && _y == p._y; };
  bool operator!=(const GUIPoint& p) const { return _x != p._x || _y != p._y; };

	long _x,_y ;
} ;
#endif
