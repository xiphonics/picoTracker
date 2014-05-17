#ifndef _INSTRUMENT_VIEW_H_
#define _INSTRUMENT_VIEW_H_

#include "Foundation/Observable.h"
#include "BaseClasses/FieldView.h"
#include "ViewData.h"

class InstrumentView: public FieldView, public I_Observer {
public:
	InstrumentView(GUIWindow &w,ViewData *data) ;
	virtual ~InstrumentView() ;

	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType,unsigned int) {} ;
	virtual void OnFocus() ;

protected:
	void warpToNext(int offset) ;
	void onInstrumentChange() ;
	void fillSampleParameters() ;
	void fillMidiParameters() ;
	InstrumentType getInstrumentType() ;
	void Update(Observable &o,I_ObservableData *d) ;

private:
	Project *project_ ;
	FourCC lastFocusID_ ;
	I_Instrument *current_ ;
} ;
#endif
