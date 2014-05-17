#ifndef _SOUND_FONT_MANAGER_H_
#define _SOUND_FONT_MANAGER_H_

#include "Foundation/T_Singleton.h"
#include "Externals/Soundfont/ENAB.H"
#include <vector>

class SoundFontManager:public T_Singleton<SoundFontManager> {
public:
	SoundFontManager() ;
	~SoundFontManager() ;
	void Reset() ;
	sfBankID LoadBank(const char *path) ;
private:
	std::vector<void *> sampleData_ ;
};
#endif
