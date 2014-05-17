
#include "GUIBitmap.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
//#include "Engine/ENGBitmap.h"
#define ENGBitmap int 
#include <stdio.h>

// This constructor. We wrap the class around an Implementation class
// That way, we can change the imps we use without changing the core code

GUIBitmap::GUIBitmap(I_GUIBitmapImp &imp) {
	_imp=&imp ;
} ;

GUIBitmap::~GUIBitmap() {
	delete _imp ;
}
// I_GUIGraphics implementation: we forward everything to the imp

void GUIBitmap::DrawLine(long x1,long y1,long x2,long y2) {
	_imp->DrawLine(x1,y1,x2,y2) ;
}

void GUIBitmap::DrawLine(GUIPoint &pt1,GUIPoint &pt2) {
	_imp->DrawLine(pt1,pt2) ;
}

void GUIBitmap::DrawRect(GUIRect &rect) {
	_imp->DrawRect(rect) ;
}

void GUIBitmap::SetColor(GUIColor &c) {
	_imp->SetColor(c) ;
}


void GUIBitmap::SelectFont(int type,int size)
{
	_imp->SelectFont(type,size) ;
}

void GUIBitmap::DrawString(char *string,GUIPoint &pos,GUITextProperties &props)
{
	_imp->DrawString(string,pos,props) ;
}

int GUIBitmap::GetStringWidth(char *string)
{
	return _imp->GetStringWidth(string) ;
}

void GUIBitmap::DrawBitmap(GUIBitmap &bmp,GUIPoint &p) 
{
	_imp->DrawBitmap(bmp,p) ;
}

void GUIBitmap::StretchBitmap(GUIBitmap &bmp,GUIRect &srcR,GUIRect &dstR) 
{
	_imp->StretchBitmap(bmp,srcR,dstR) ;
}

void GUIBitmap::StretchENGBitmap(ENGBitmap &bmp,GUIRect &srcR,GUIRect &dstR) 
{
	_imp->StretchENGBitmap(bmp,srcR,dstR) ;
}

GUIRect GUIBitmap::GetRect() {
	return _imp->GetRect() ;
}

void GUIBitmap::SetClipRect(GUIRect &r) {
	_imp->SetClipRect(r) ;
}

void GUIBitmap::Invalidate() {
	_imp->Invalidate() ;
}

void GUIBitmap::Flush() {
	_imp->Invalidate() ;
}

void GUIBitmap::SetRect(GUIRect &r) {
	_imp->SetRect(r) ;
}
// Static call to create bitmaps from resources. We rely on the
// factory object to create an imp compatible with the system and
// then wrap a GUIBitmap object around it.

GUIBitmap *GUIBitmap::CreateFromResource(GUIResourceID &r) {
	I_GUIBitmapImp *imp=I_GUIWindowFactory::GetInstance()->CreateBitmapFromResource(r) ;
	if (imp) {
		return new GUIBitmap(*imp) ;
	} else {
		return NULL ;
	}
}

GUIBitmap *GUIBitmap::CreateBitmap(int width,int height) {

	I_GUIBitmapImp *imp=I_GUIWindowFactory::GetInstance()->CreateBitmap(width,height) ;
	if (imp) {
		return new GUIBitmap(*imp) ;
	} else {
		return NULL ;
	}
}
