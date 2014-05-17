
#include "GUIRect.h"

// Constructor: specifies top,lef,bottom and right coordinates

GUIRect::GUIRect(long x0,long y0,long x1,long y1):_topLeft(x0,y0),_bottomRight(x1,y1) {
}

GUIRect::GUIRect(GUIPoint& topLeft,GUIPoint& bottomRight):_topLeft(topLeft),_bottomRight(bottomRight) {
}

// Returns true if the point is contained inside the specified rectangle

bool GUIRect::Contains(GUIPoint &p) {
	return p._x>=_topLeft._x && p._x<=_bottomRight._x && p._y>=_topLeft._y && p._y<=_bottomRight._y ;
}

// Returns the topLeft corner of the rectangle

GUIPoint GUIRect::GetPosition() {
	return _topLeft ;
}

// Moves the rectangle to the specified topLeft point. The rectangle keeps
// the same size

void GUIRect::SetPosition(GUIPoint &point) {
	long w=_bottomRight._x-_topLeft._x ;
	long h=_bottomRight._y-_topLeft._y ;
	_topLeft=point ;
	_bottomRight=point ;
	_bottomRight.Add(GUIPoint(w,h)) ;
}

// Translate the rectangle

void GUIRect::Translate(GUIPoint &p) {
	_topLeft.Add(p) ;
	_bottomRight.Add(p) ;
}

GUIRect GUIRect::Intersect(GUIRect &other) {
	this->Normalize() ;
	other.Normalize() ;

	GUIPoint topLeft=_topLeft ;
	if (other.Left()>topLeft._x) {
		topLeft._x=other.Left() ;
	}
	if (other.Top()>topLeft._y) {
		topLeft._y=other.Top() ;
	}

	GUIPoint bottomRight=_bottomRight ;
	if (other.Right()<bottomRight._x) {
		bottomRight._x=other.Right() ;
	}
	if (other.Bottom()<bottomRight._y) {
		bottomRight._y=other.Bottom() ;
	}
	return GUIRect(topLeft,bottomRight) ;
}

void GUIRect::Normalize() {
	if (_topLeft._x>_bottomRight._x) {
		int x=_topLeft._x ;
		_topLeft._x=_bottomRight._x ;
		_bottomRight._x=x ;
	}
	if (_topLeft._y>_bottomRight._y) {
		int y=_topLeft._y ;
		_topLeft._y=_bottomRight._y ;
		_bottomRight._y=y ;
	}
}
