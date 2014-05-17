#ifndef _I_GUIGRAPHICS_H_
#define _I_GUIGRAPHICS_H_

#include "UIFramework/Framework/GUIColor.h"
#include "UIFramework/BasicDatas/GUIRect.h"
#include "UIFramework/Framework/GUITextProperties.h"
//#include "Engine/ENGBitmap.h"


// Interface definition for a graphical port.

class I_GUIGraphics {
public:
	virtual ~I_GUIGraphics() {} ;
	virtual void Clear(GUIColor &,bool overlay=false)=0 ;
	virtual void SetColor(GUIColor &)=0 ;
	virtual void ClearRect(GUIRect &)=0 ;
	virtual void DrawString(const char *string,GUIPoint &pos,GUITextProperties& p,bool overlay)=0 ;
	virtual void DrawChar(const char c,GUIPoint &pos,GUITextProperties &)=0;

	virtual GUIRect GetRect()=0 ;
	virtual void Invalidate()=0 ;
	virtual void Lock()=0 ;
	virtual void Unlock()=0 ;
	virtual void Flush()=0 ;
} ;

#endif
