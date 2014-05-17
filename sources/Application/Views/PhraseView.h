
#ifndef _PHRASE_VIEW_H_
#define _PHRASE_VIEW_H_

#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/View.h"
#include "ViewData.h"

#define FCC_EDIT MAKE_FOURCC('V','O','L','M')

class PhraseView: public View {

public:

	PhraseView(GUIWindow &w,ViewData *viewData) ;
	~PhraseView() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int tick=0) ;
	virtual void OnFocus() ;
protected:
	void updateCursor(int dx,int dy) ;
	void updateCursorValue(ViewUpdateDirection offset,int xOffset=0,int yOffset=0) ;
	void updateSelectionValue(ViewUpdateDirection direction) ;
	void warpToNeighbour(int offset) ;
	void warpInChain(int offset) ;
	void cutPosition() ;
	void pasteLast() ;

    void extendSelection() ;
    
	GUIRect getSelectionRect() ;
	void fillClipboardData() ;
	void copySelection() ;
	void cutSelection() ;
	void pasteClipboard() ;

	void unMuteAll() ;
	void toggleMute() ;
	void switchSoloMode() ;

	void processNormalButtonMask(unsigned short mask) ;
	void processSelectionButtonMask(unsigned short mask) ;

    void setTextProps(GUITextProperties & props, int row,int col,bool restore) ;

private:

	int row_ ;
	int col_ ;
	int lastNote_ ;
	int lastInstr_ ;
	int lastCmd_ ;
	int lastParam_ ;
	Phrase *phrase_ ;
	int lastPlayingPos_ ;
	Variable cmdEdit_ ;
	UIBigHexVarField *cmdEditField_ ;

	struct clipboard {
		bool active_ ;
		int col_ ;
		int row_ ;
		int width_ ;
		int height_ ;
		uchar note_[16] ;
		uchar instr_[16] ;
		uint cmd1_[16] ;
 		ushort param1_[16] ;
		uint cmd2_[16] ;
 		ushort param2_[16] ;
	} clipboard_ ;

	int saveCol_ ;
	int saveRow_ ;

	static short offsets_[2][4] ;
} ;

#endif
