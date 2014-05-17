#ifndef DEB_GUI_WINDOW_H_
#define DEB_GUI_WINDOW_H_

#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include <SDL/SDL.h>

bool ProcessDEBEvent(SDL_Event &event) ;
void ProcessButtonChange(unsigned short,unsigned short) ;

#define ENGBitmap int

#define DEB_BUTTON_A 0x1
#define DEB_BUTTON_B 0x2
#define DEB_BUTTON_LEFT 0x4
#define DEB_BUTTON_B 0x2
#define DEB_BUTTON_RIGHT 0x8
#define DEB_BUTTON_UP 0x10
#define DEB_BUTTON_DOWN 0x20
#define DEB_BUTTON_L 0x40
#define DEB_BUTTON_R 0x80
#define DEB_BUTTON_START 0x100
#define DEB_BUTTON_SELECT 0x200

class DEBGUIWindowImp: public I_GUIWindowImp {

public:

	DEBGUIWindowImp(GUICreateWindowParams &p) ;
	virtual ~DEBGUIWindowImp() ;

public: // I_GUIWindowImp implementation

	virtual void SetColor(GUIColor &) ;
	virtual void DrawRect(GUIRect &) ;
	virtual void DrawString(const char *string,GUIPoint &pos,GUITextProperties &);
	virtual GUIRect GetRect() ;
	virtual void Invalidate() ;
	virtual void Save() ;
	virtual void Restore() ;
	virtual void Flush();
	virtual void Clear(GUIColor &) ;
	virtual void ClearRect(GUIRect &) ;

public: // Added functionality
	bool ProcessDEBEvent(SDL_Event &event) ;
	void ProcessButtonChange(unsigned short sendMask,unsigned short mask) ;
	void SendEscape() ;
protected:
	void prepareFonts() ;
private:
	SDL_Surface *screen_;
	SDL_Surface *offscreen_ ;
	GUIRect rect_ ;
	SDLMod lastMod_ ;
	unsigned int currentColor_ ;
	unsigned int backgroundColor_ ;
} ;
#endif
