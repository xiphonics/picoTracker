#ifndef NC_GUI_WINDOW_H_
#define NC_GUI_WINDOW_H_

#include "UIFramework/Interfaces/I_GUIWindowImp.h"


class NCGUIWindowImp: public I_GUIWindowImp {

public:

	NCGUIWindowImp(GUICreateWindowParams &p) ;
	virtual ~NCGUIWindowImp() ;

public: // I_GUIWindowImp implementation

	virtual void SetColor(GUIColor &) ;
	virtual void DrawRect(GUIRect &) ;
	virtual void DrawChar(const char c,GUIPoint &pos,GUITextProperties &);
	virtual void DrawString(const char *string,GUIPoint &pos,GUITextProperties &,bool overlay=false);
	virtual GUIRect GetRect() ;
	virtual void Invalidate() ;
	virtual void Flush();
	virtual void Lock() ;
	virtual void Unlock() ;
	virtual void Clear(GUIColor &, bool overlay=false) ;
	virtual void ClearRect(GUIRect &) ;
	virtual void PushEvent(GUIEvent &event) ;
	void ProcessExpose() ;

/*public: // Added functionality
	void ProcessQuit() ;
	void ProcessUserEvent(SDL_Event &event) ;
*/
private:
} ;
#endif
