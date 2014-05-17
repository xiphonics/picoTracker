#ifndef WSDL_GUI_WINDOW_H_
#define WSDL_GUI_WINDOW_H_

#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include "Externals/SDL/SDL.h"

bool ProcessWSDLEvent(SDL_Event &event) ;
void ProcessButtonChange(unsigned short,unsigned short) ;

#define ENGBitmap int

#define WSDL_BUTTON_A 0x1
#define WSDL_BUTTON_B 0x2
#define WSDL_BUTTON_LEFT 0x4
#define WSDL_BUTTON_RIGHT 0x8
#define WSDL_BUTTON_UP 0x10
#define WSDL_BUTTON_DOWN 0x20
#define WSDL_BUTTON_L 0x40
#define WSDL_BUTTON_R 0x80
#define WSDL_BUTTON_START 0x100
#define WSDL_BUTTON_SELECT 0x200

class WSDLGUIWindowImp: public I_GUIWindowImp {

public:

	WSDLGUIWindowImp(GUICreateWindowParams &p) ;
	virtual ~WSDLGUIWindowImp() ;

public: // I_GUIWindowImp implementation

	virtual void SetColor(GUIColor &) ;
	virtual void ClearRect(GUIRect &) ;
	virtual void DrawString(const char *string,GUIPoint &pos,GUITextProperties &);
	virtual GUIRect GetRect() ;
	virtual void Invalidate() ;
	virtual void Save() ;
	virtual void Restore() ;
	virtual void Flush();
	virtual void Clear(GUIColor &) ;

public: // Added functionality
	bool ProcessWSDLEvent(SDL_Event &event) ;
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
