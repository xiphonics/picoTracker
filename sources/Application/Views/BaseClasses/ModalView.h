
#ifndef _MODAL_VIEW_H_
#define _MODAL_VIEW_H_

#include "View.h"

class ModalView: public View {
public:
	ModalView(View &) ;
	virtual ~ModalView() ;

	bool IsFinished() ;
	int GetReturnCode() ;

protected:
	void SetWindow(int width,int height) ;
	virtual void ClearRect(int x,int y,int w,int h) ;
	virtual void DrawString(int x,int y,const char *txt,GUITextProperties &props) ;
	void EndModal(int returnCode) ;
private:
	bool finished_ ;
	int returnCode_ ;
	int left_ ;
	int top_ ;
} ;
#endif