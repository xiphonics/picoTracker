#include "TimeService.h"
#include "System/System/System.h"
#include "SDL/SDL.h"

/*Date::Date() {
} ;

Date::Date(const wchar_t *string,const wchar_t *format) {
	date_.ParseFormat((wxChar *)string,(wxChar *)format) ;
} ;

std::wstring Date::FormatDate(const wchar_t *format) {
	wxString string=date_.Format((wxChar *)format) ;
	return std::wstring((wchar_t *)string.c_str()) ;
} ;

Date::~Date() {
} ;

Date Date::Now() {
	Date d ;
	d.date_=wxDateTime::Now() ;
	return d ;
};

bool Date::IsEarlierThan(const Date &other) {
	return date_.IsEarlierThan(other.date_) ;
} ;
*/


TimeService::TimeService() {
	startTick_=System::GetInstance()->GetClock() ;
} ;

TimeService::~TimeService() {
} ;

Time TimeService::GetTime() {
	unsigned long currentTick=System::GetInstance()->GetClock()  ;
	return (currentTick-startTick_)/1000.0 ;
} ;

void TimeService::Sleep(int msecs) {
	SDL_Delay(msecs) ;
} ;