#ifndef _TABLE_VIEW_H_
#define _TABLE_VIEW_H_

#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/View.h"
#include "ViewData.h"
#include "Application/Model/Table.h"

class TableView: public View {
public:
	TableView(GUIWindow &w,ViewData *viewData) ;
	~TableView() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int tick=0) ;
	virtual void OnFocus() ;


protected:

	void processNormalButtonMask(unsigned short mask) ;
	void processSelectionButtonMask(unsigned short mask) ;

	void cutPosition() ;
	void pasteLast() ;
	void copySelection() ;
	void cutSelection() ;
	void pasteClipboard() ;

	void fillClipboardData() ;
	void extendSelection() ;

	void updateCursor(int dx,int dy) ;
	void updateCursorValue(int offset) ;
	void setTextProps(GUITextProperties &props,int row,int col,bool restore) ;
	void warpToNeighbour(int dir) ;

	GUIRect getSelectionRect() ;

private:
	int row_ ;
	int col_ ;
	uchar lastVol_ ;
	uchar lastTick_ ;
	uchar lastTsp_ ;
	int lastCmd_ ;
	int lastParam_ ;

	Variable cmdEdit_ ;
	UIBigHexVarField *cmdEditField_ ;

	struct clipboard {
		bool active_ ;
		int col_ ;
		int row_ ;
		int width_ ;
		int height_ ;
		uint cmd1_[16] ;
 		ushort param1_[16] ;
		uint cmd2_[16] ;
 		ushort param2_[16] ;
		uint cmd3_[16] ;
 		ushort param3_[16] ;
	} clipboard_ ;

	uchar saveCol_ ;
	uchar saveRow_ ;

	uchar lastPosition_[3] ;
} ;


#endif
