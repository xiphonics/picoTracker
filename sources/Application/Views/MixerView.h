#ifndef _MIXER_VIEW_H_
#define _MIXER_VIEW_H_

#include "BaseClasses/View.h"
#include "ViewData.h"

class MixerView: public View {
public:
	MixerView(GUIWindow &w,ViewData *viewData) ;
	~MixerView() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int tick=0) ;
	virtual void OnFocus() ;
protected:
	void processNormalButtonMask(unsigned int mask) ;
    void processSelectionButtonMask(unsigned int mask) ;
	void onStart() ;
	void onStop() ;
	void updateCursor(int dx,int dy)  ;
private:
	const char *song_ ;
	int saveX_ ;
	int saveY_ ;
	int saveOffset_ ;
	bool invertBatt_ ;
} ;
#endif