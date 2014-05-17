
#ifndef _GUIPARENTGRAPHICS_H_
#define _GUIPARENTGRAPHICS_H_

#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "UIFramework/Interfaces/I_GUIControl.h"

#include <stdio.h>

class GUIParentGraphics: public I_GUIGraphics {
public:

	GUIParentGraphics() ;
	virtual ~GUIParentGraphics() ;

	void SetControl(I_GUIControl &control) ;

	void ParentChanged() ;

	// Implementation of I_GUIGraphics. All coordinates are specified
	// with respect to the local control.

	virtual void DrawLine(long,long,long,long) ;
	virtual void DrawLine(GUIPoint&,GUIPoint&) ;
	virtual void SetColor(GUIColor &) ;
	virtual void DrawRect(GUIRect &) ;
	virtual void DrawBitmap(GUIBitmap &,GUIPoint &p) ;
	virtual void StretchBitmap(GUIBitmap &,GUIRect &srcR,GUIRect &dstR) ;
	virtual void StretchENGBitmap(ENGBitmap &,GUIRect &srcR,GUIRect &dstR) ;
	virtual void SelectFont(int type,int size);
	virtual void DrawString(char *string,GUIPoint &pos,GUITextProperties &);
	virtual int	 GetStringWidth(char *string);
	virtual void SetClipRect(GUIRect &r) ;
	virtual void Invalidate() ;
	virtual GUIRect GetRect() ;
	virtual void SetRect(GUIRect &) ;

	// Returns the event services associated to the control

	I_GUIEventService *GetEventService() ;

	// Returns the graphics used to render the control

	I_GUIGraphics *GetGraphics() ;

	// Backing store enabling

	void UseBackingStore(bool) ;
	bool UseBackingStore() ;

	// Backing store flush. NOP if bs not enabled

	void Flush() ;

private:
	void UpdateBSSpace() ;

private:
	I_GUIControl *_control ;
	I_GUIGraphics *_graphicDC ;
	I_GUIEventService *_eventService ;
	GUIBitmap *_bs ;
	bool _useBs ;
} ;

#endif
