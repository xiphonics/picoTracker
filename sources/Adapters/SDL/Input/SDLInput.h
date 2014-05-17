
#ifndef _SDL_INPUT_H_
#define _SDL_INPUT_H_

#include "Foundation/T_Singleton.h"
#include "Externals/SDL/SDL.h"

#define SDLI_BUTTON_A 0x1
#define SDLI_BUTTON_B 0x2
#define SDLI_BUTTON_LEFT 0x4
#define SDLI_BUTTON_RIGHT 0x8
#define SDLI_BUTTON_UP 0x10
#define SDLI_BUTTON_DOWN 0x20
#define SDLI_BUTTON_L 0x40
#define SDLI_BUTTON_R 0x80
#define SDLI_BUTTON_START 0x100
#define SDLI_BUTTON_SELECT 0x200


class SDLInput: public T_Singleton<SDLInput> {
public:
	SDLInput() ;
	void ReadConfig();
	unsigned short GetButtonMask() ;

private:
	void mapKey(int index,const char *keyname) ;

	SDL_Joystick *joystick_;

	const char *keyname_[SDLK_LAST] ;

} ;

#endif