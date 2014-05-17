#ifndef _GUICOLOR_H_
#define _GUICOLOR_H_

// A simple RGB Color representation class

class GUIColor {
public:
	GUIColor(unsigned short r,unsigned short g,unsigned short b) {
		_r=r ;_g=g ; _b=b ;
	}
	unsigned short _r,_g,_b ;
} ;
#endif
