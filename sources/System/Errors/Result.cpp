#include "Result.h"
#include <assert.h>

Result Result::NoError ;

Result::Result()
:success_(true)
,error_("success")
,checked_(true)
,child_(0){}

Result::Result(const std::string &error)
:success_(false)
,error_(error)
,checked_(false)
,child_(0) {}

Result::Result(const std::ostringstream &error)
:success_(false)
,error_(error.str())
,checked_(false)
,child_(0) {}

Result::Result(Result &cause,const std::string &error)
:success_(false)
,error_(error)
,checked_(false)
,child_(new Result(cause))
{
  child_->checked_ = true;
	cause.checked_ = true;
}

Result::Result(const Result &other) {
	success_=other.success_ ;
	error_=other.error_ ;
	checked_ = false ;
	other.checked_ = true ;
	child_ = other.child_ ;
	other.child_ = 0 ;
}

Result::~Result() {
	NAssert(checked_) ;
	delete(child_) ;
}

Result &Result::operator =(const Result &other) {
	success_=other.success_ ;
	error_=other.error_ ;
	checked_ = false ;
	other.checked_ = true ;
	child_ = other.child_ ;
	other.child_=0 ;
	return *this;
}

bool Result::Failed() {
	checked_= true ;
	return !success_ ;
}

bool Result::Succeeded() {
	checked_= true ;
	return success_ ;
}

std::string Result::GetDescription() {
	std::string description = error_ ;
	if (child_) {
		description+="\n>>>> " ;
		description+=child_->GetDescription() ;
	}
	return description ;
}
