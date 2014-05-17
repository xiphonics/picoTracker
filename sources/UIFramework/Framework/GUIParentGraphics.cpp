
#include "GUIParentGraphics.h"
#include "UIFramework/BasicDatas/GUIBitmap.h"
#include "Application/utils/assert.h"

GUIParentGraphics::GUIParentGraphics()
{
  _graphicDC=NULL ;
  _eventService=NULL ;
	_bs=NULL ;
	_useBs=false ;
}

GUIParentGraphics::~GUIParentGraphics() {
	if (_bs) {
		delete(_bs) ;
	}
}

void GUIParentGraphics::ParentChanged() {
  _graphicDC=NULL ;
  _eventService=NULL ;
}

void GUIParentGraphics::SetControl(I_GUIControl &control) {
  _control=&control ;
  ParentChanged() ;
}

// Returns the Graphic device used to render the control

I_GUIGraphics *GUIParentGraphics::GetGraphics() {
	if (_graphicDC==NULL) {
		_graphicDC=_control->GetParent()->GetDC() ;
	}
	return _graphicDC ;
}

// Returns the Event service available to the control

I_GUIEventService *GUIParentGraphics::GetEventService() {
	if (_eventService==NULL) {
		_eventService=_control->GetParent()->GetEventService() ;
	}
	return _eventService ;
}

// I_GUIGraphics implementation: The control can draw on the graphical
// device of its parent by specifying coordinates in its own system

void GUIParentGraphics::DrawLine(long x1,long y1,long x2,long y2) {

	if (_useBs) {
		_bs->DrawLine(x1,y1,x2,y2) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		GUIPoint p=_control->GetParent()->GetOffset() ;
		p.Add(_control->GetPosition()) ;
		graphicDC->DrawLine(x1+p._x,y1+p._y,x2+p._x,y2+p._y) ;
	}
}

void GUIParentGraphics::DrawLine(GUIPoint &pt1,GUIPoint &pt2) 
{
	DrawLine(pt1._x,pt1._y,pt2._x,pt2._y);
}

void GUIParentGraphics::SetColor(GUIColor &color) {
	if ( _useBs) {
		_bs->SetColor(color) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		graphicDC->SetColor(color) ;
	}
}

void GUIParentGraphics::DrawRect(GUIRect &rect) {
	if (_useBs) {
		_bs->DrawRect(rect) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		GUIRect r=rect ;
		GUIPoint p=r.GetPosition() ;
		p.Add(_control->GetOffset()) ;
		r.SetPosition(p) ;
		graphicDC->DrawRect(r) ;
	}
}

void GUIParentGraphics::DrawBitmap(GUIBitmap &b,GUIPoint &p) {
	if (_useBs) {
		_bs->DrawBitmap(b,p) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		p.Add(_control->GetOffset()) ;
		graphicDC->DrawBitmap(b,p) ;
	}
}

void GUIParentGraphics::StretchBitmap(GUIBitmap &bmp,GUIRect &srcR,GUIRect &dstR) {

	if (_useBs) {
		_bs->StretchBitmap(bmp,srcR,dstR) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		GUIRect dstRect=dstR ;
		GUIRect srcRect=srcR ;
		GUIPoint offset=_control->GetOffset();
		dstRect.Translate(offset) ;
	//	srcRect.Translate(_control->GetOffset()) ;
		graphicDC->StretchBitmap(bmp,srcRect,dstRect) ;
	}
}

void GUIParentGraphics::StretchENGBitmap(ENGBitmap &bmp,GUIRect &srcR,GUIRect &dstR) {

	if (_useBs) {
		_bs->StretchENGBitmap(bmp,srcR,dstR) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		GUIRect dstRect=dstR ;
		GUIRect srcRect=srcR ;
		GUIPoint offset=_control->GetOffset();
		dstRect.Translate(offset) ;
	//	srcRect.Translate(_control->GetOffset()) ;
		graphicDC->StretchENGBitmap(bmp,srcRect,dstRect) ;
	}
}

void GUIParentGraphics::SelectFont(int type,int size) 
{
	if (_useBs) {
		_bs->SelectFont(type,size) ;
	} else 
	{
		I_GUIGraphics *graphicDC=GetGraphics() ;
		graphicDC->SelectFont(type,size) ;
	}
}

int GUIParentGraphics::GetStringWidth(char *string) 
{
	if (_useBs) {
		return _bs->GetStringWidth(string) ;
	} else 
	{
		I_GUIGraphics *graphicDC=GetGraphics() ;
		return graphicDC->GetStringWidth(string) ;
	}
}

void GUIParentGraphics::DrawString(char *string,GUIPoint &pos,GUITextProperties &props)
{
	if (_useBs) {
		_bs->DrawString(string,pos,props) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		GUIPoint p=pos ;
		p.Add(_control->GetOffset()) ;
		graphicDC->DrawString(string,p,props) ;
	}
}


void GUIParentGraphics::SetClipRect(GUIRect &rect) {
	if (_useBs) {
		_bs->SetClipRect(rect) ;
	} else {
		I_GUIGraphics *graphicDC=GetGraphics() ;
		GUIRect r=rect ;
		GUIPoint p=r.GetPosition() ;
		p.Add(_control->GetOffset()) ;
		r.SetPosition(p) ;
		graphicDC->SetClipRect(r) ;
	}
}

void GUIParentGraphics::Invalidate() {
	I_GUIGraphics *graphicDC=GetGraphics() ;
	graphicDC->Invalidate() ;
}

GUIRect GUIParentGraphics::GetRect() {
	return _control->GetRect() ;
}

void GUIParentGraphics::SetRect(GUIRect &) {
	if (_useBs) {
		UpdateBSSpace() ;
	}
}

void GUIParentGraphics::UpdateBSSpace() {
	n_assert(_bs!=NULL) ;
	GUIRect r=GetRect() ;
	_bs->SetRect(r) ;
}

void GUIParentGraphics::UseBackingStore(bool useit) {
	if (useit) {
		_useBs=true ;
        if (_bs==NULL) {
			GUIRect r=GetRect() ;
			_bs=GUIBitmap::CreateBitmap(r.Width(),r.Height()) ;
		} else {
			UpdateBSSpace() ;
		}
	} else {
		_useBs=false ;
	}
}

bool GUIParentGraphics::UseBackingStore() {
	return _useBs ;
}

void GUIParentGraphics::Flush() {
	if (_useBs&&_bs) {
		I_GUIControl *parent=_control->GetParent() ;
		GUIPoint zero ;
		parent->GetDC()->DrawBitmap(*_bs,zero) ;
	}
}
