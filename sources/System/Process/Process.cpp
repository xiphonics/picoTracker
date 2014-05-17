
#include "Process.h"
#include "System/Console/Trace.h"
#include "System/Console/n_assert.h"

bool SysThread::Start() {
	return SysProcessFactory::GetInstance()->BeginThread(*this) ;
} ;

bool SysThread::startExecution() {
	isFinished_=false ;
	shouldTerminate_=false ;
	bool result=false ;
	try {
			result=Execute() ;
	} catch (...) {
		NInvalid ;
	}
	isFinished_=true ;
	return result ;
}

bool SysThread::IsFinished() {
	return isFinished_ ;
}

bool SysThread::shouldTerminate() {
	return shouldTerminate_ ;
}

void SysThread::RequestTermination() {
	shouldTerminate_=true ;
}


SysSemaphore *SysSemaphore::Create(int initialcount,int maxcount) {
	return SysProcessFactory::GetInstance()->CreateNewSemaphore(initialcount,maxcount) ;
} ;

