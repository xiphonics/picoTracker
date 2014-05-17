
#include "GUIWindow.h"
#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"

// Constructor: We wrap the window around an implementation that
// will be used to provide system functionalities

GUIWindow::GUIWindow(I_GUIWindowImp &imp) {
	_imp=&imp ;
	_imp->SetWindow(*this) ; // We tell the imp on which windows it works so
	                         // we can be notified of system events
}

// Destructor

GUIWindow::~GUIWindow() {
	delete _imp ;
}

// I_GUIGraphics Implementation: We rely on the imp window to provide
// core graphics on the window

void GUIWindow::SetColor(GUIColor &color) {
	_imp->SetColor(color) ;
}

void GUIWindow::ClearRect(GUIRect &r) {
	_imp->ClearRect(r) ;
}

void GUIWindow::DrawString(const char *string,GUIPoint &pos,GUITextProperties &props,bool overlay)
{
	_imp->DrawString(string,pos,props,overlay) ;
}

void GUIWindow::DrawChar(const char c,GUIPoint &pos,GUITextProperties &props) {
	_imp->DrawChar(c,pos,props) ;
}

void GUIWindow::Clear(GUIColor &c,bool overlay) {
	_imp->Clear(c,overlay) ;
} ;

/*void GUIWindow::Save() {
	_imp->Save() ;
} ;

void GUIWindow::Restore() {
	_imp->Restore() ;
} ;*/

GUIRect GUIWindow::GetRect() {
	return _imp->GetRect() ;
}

void GUIWindow::Invalidate() {
	_imp->Invalidate() ;
}

void GUIWindow::Flush() {
	_imp->Flush() ;
}

void GUIWindow::Lock() {
	_imp->Lock() ;
}
void GUIWindow::Unlock() {
	_imp->Unlock() ;
}

void GUIWindow::Update() {
	onUpdate() ;
}


I_GUIGraphics *GUIWindow::GetGraphics() {
	return this ;
}

I_GUIGraphics *GUIWindow::GetDC() {
	return this ;
}

// Redifine the event Dispatcher to handle focused controll

bool GUIWindow::DispatchEvent(GUIEvent &event) 
{
	return onEvent(event) ;;
}

void GUIWindow::PushEvent(GUIEvent &event) {
	_imp->PushEvent(event) ;
} ;