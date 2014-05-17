#ifndef WSDL_GUI_WINDOW_H_
#define WSDL_GUI_WINDOW_H_

#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include "Adapters/NDS/System/NDSEventQueue.h"
#include "System/System/typedefs.h"

class NDSGUIWindowImp: public I_GUIWindowImp {

public:

	NDSGUIWindowImp(GUICreateWindowParams &p) ;
	virtual ~NDSGUIWindowImp() ;

public: // I_GUIWindowImp implementation

	virtual void SetColor(GUIColor &) ;
	virtual void DrawString(char *string,GUIPoint &pos,GUITextProperties &);
	virtual GUIRect GetRect() ;
	virtual void Invalidate() ;
	virtual void Save() ;
	virtual void Restore() ;
	virtual void Flush();
	virtual void Clear(GUIColor &) ;
	
    static void ProcessEvent(NDSEvent &event) ;	
    static void ProcessButtonChange(uint16 changeMask,uint16 buttonMask);
private:
    unsigned short* framebuffer_;    
    unsigned short frontColor_ ;
    unsigned short backColor_ ;
    GUIRect rect_ ;
} ;
#endif
