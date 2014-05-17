#ifndef CAANOO_GUI_WINDOW_H_
#define CAANOO_GUI_WINDOW_H_

#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include <SDL/SDL.h>
#include "Adapters/CAANOOSystem/CAANOOEventQueue.h"

bool ProcessCAANOOEvent(CAANOOEvent &event) ;
void ProcessButtonChange(unsigned short,unsigned short) ;
void SendESCMessage() ;

#define ENGBitmap int

#define PIGGY_BUTTON_A 0x1
#define PIGGY_BUTTON_B 0x2
#define PIGGY_BUTTON_LEFT 0x4
#define PIGGY_BUTTON_RIGHT 0x8
#define PIGGY_BUTTON_UP 0x10
#define PIGGY_BUTTON_DOWN 0x20
#define PIGGY_BUTTON_L 0x40
#define PIGGY_BUTTON_R 0x80
#define PIGGY_BUTTON_START 0x100
#define PIGGY_BUTTON_SELECT 0x200

class CAANOOGUIWindowImp: public I_GUIWindowImp {

public:

	CAANOOGUIWindowImp(GUICreateWindowParams &p) ;
	virtual ~CAANOOGUIWindowImp() ;

public: // I_GUIWindowImp implementation

	virtual void SetColor(GUIColor &) ;
	virtual void DrawRect(GUIRect &) ;
	virtual void DrawString(const char *string,GUIPoint &pos,GUITextProperties &);
	virtual GUIRect GetRect() ;
	virtual void Invalidate() ;
    virtual void Flush();
	virtual void Clear(GUIColor &) ;
	virtual void Save() ;
	virtual void Restore() ;
    virtual void ClearRect(GUIRect &r) ;
public: // Added functionality
	bool ProcessCAANOOEvent(CAANOOEvent &event) ;
	void ProcessButtonChange(unsigned short sendMask,unsigned short mask) ;
	void SendESCMessage() ;
protected:
	void prepareFonts() ;
private:
	SDL_Surface *screen_;
	SDL_Surface *offscreen_;
	GUIRect rect_ ;
	SDLMod lastMod_ ;
	unsigned int currentColor_ ;
	unsigned int backgroundColor_ ;
	unsigned int foregroundColor_ ;
} ;
#endif
