
#ifndef _GUIRECT_H_
#define _GUIRECT_H_

#include "GUIPoint.h"

// A Simple class to represent rectangles. Note that the Constructor
// is using top,left,bottom,right and NOT x,y coordinates

class GUIRect 
{
public:

	GUIRect(long x0=0,long y0=0,long x1=0,long y1=0) ;
	GUIRect(GUIPoint &,GUIPoint &) ;

	// Returns true if the specified point is contained in the rectangle.
	// Include completely the rectangle's border

	bool Contains(GUIPoint &) ;

	// Moves the rectangle top-left position keeping its width/height

	void SetPosition(GUIPoint &) ;

	// Returns the top-left corner of the rectangle

	GUIPoint GetPosition() ;

	// Returns a rectangle resulting of the intersection of the two rects

	GUIRect Intersect(GUIRect &) ;

	// Make sure the top/left/right/bottom are in correct order

	void Normalize() ;

	// Translate the rectangle from the given offset

	void Translate(GUIPoint &) ;

	// Accessor to the rectangle coordinates and size

	inline int Top() const { return _topLeft._y ; }
	inline int Left() const { return _topLeft._x ; }
	inline int Bottom() const { return _bottomRight._y ; }
	inline int Right() const { return _bottomRight._x ; }
	inline int Width() const { return _bottomRight._x-_topLeft._x ; }
	inline int Height() const { return _bottomRight._y-_topLeft._y; }


public:
	GUIPoint _topLeft ;     // Top left Corner
	GUIPoint _bottomRight ; // Bottom Right Corner
} ;

#endif
