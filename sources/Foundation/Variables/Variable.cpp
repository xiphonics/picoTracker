#include "Variable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include "System/Console/n_assert.h"
#include "System/Console/Trace.h"

Variable::Variable(const char *name,FourCC id,float value) {
	name_=name ;
	id_=id ;
	value_.float_=value ;
	defaultValue_.float_=value ;
	type_=FLOAT ;
} ;

Variable::Variable(const char *name,FourCC id,int value) {
	name_=name ;
	id_=id ;
	value_.int_=value ;
	defaultValue_.int_=value ;
	type_=INT ;
} ;

Variable::Variable(const char *name,FourCC id,bool value) {
	name_=name ;
	id_=id ;
	value_.bool_=value ;
	defaultValue_.bool_=value ;
	type_=BOOL ;
} ;

Variable::Variable(const char *name,FourCC id,const char *value) {
	name_=name ;
	id_=id ;
	stringValue_=value ;
	stringDefaultValue_=value ;
	type_=STRING ;
} ;

Variable::Variable(const char *name,FourCC id,char **list,int size,int index) {
	name_=name ;
	id_=id ;
	list_.char_=list ;
	listSize_=size ;
	value_.index_=index ;
	defaultValue_.index_=index ;
	type_=CHAR_LIST ;
} ;

Variable::~Variable() {
} ;

Variable::Type Variable::GetType() {
	return type_ ;
} ;

FourCC Variable::GetID() {
	return id_ ;
} ;

const char* Variable::GetName() {
	return name_.c_str() ;
} ;

void Variable::SetFloat(float value,bool notify) {
	switch(type_) {
		case FLOAT:
			value_.float_=value ;
			break ;
		case INT:
			value_.int_=int(value);
			break ;
		case BOOL:
			value_.bool_=bool(value!=0) ;
			break ;
		case CHAR_LIST:
			value_.index_=int(value) ;
			break ;
		case STRING:
			sprintf(string_,"%f",value) ;
			stringValue_=string_ ;
			break ;
	} ;
	if (notify) {
    	onChange();
    }
} ;

void Variable::SetInt(int value,bool notify) {
	switch(type_) {
		case FLOAT:
			value_.float_=float(value) ;
			break ;
		case INT:
			value_.int_=value;
			break ;
		case BOOL:
			value_.bool_=bool(value!=0) ;
			break ;
		case CHAR_LIST:
			value_.index_=value ;
			break ;
		case STRING:
			sprintf(string_,"%d",value) ;
			stringValue_=string_ ;
			break ;

	} ;
	if (notify) {
    	onChange();
    }
} ;

void Variable::SetBool(bool value,bool notify) {
	switch(type_) {
		case FLOAT:
			value_.float_=float(value) ;
			break ;
		case INT:
			value_.int_=int(value) ;
			break ;
		case BOOL:
			value_.bool_=value ;
			break ;
		case CHAR_LIST:
			value_.index_=int(value) ;
			break ;
		case STRING:
			stringValue_=(value)?"true":"false" ;
			break ;
	} ;
	if (notify) {
    	onChange();
    }
} ;

float Variable::GetFloat() {
	switch(type_) {
		case FLOAT:
			return value_.float_ ;
		case INT:
			return float(value_.int_);
		case BOOL:
			return float(value_.bool_) ;
		case CHAR_LIST:
			return float(value_.index_);
		case STRING:
			return float(atof(stringValue_.c_str())) ;
	} ;
	return 0.0f ;
} ;

int Variable::GetInt() {
	switch(type_) {
		case FLOAT:
			return int(value_.float_) ;
		case INT:
			return value_.int_;
		case BOOL:
			return int(value_.bool_) ;
		case CHAR_LIST:
			return value_.index_;
		case STRING:
			return atoi(stringValue_.c_str()) ;
	} ;
	return 0 ;
} ;

bool Variable::GetBool() {
	switch(type_) {
		case FLOAT:
			return bool(value_.float_!=0) ;
		case INT:
			return bool(value_.int_!=0);
		case BOOL:
			return value_.bool_ ;
		case CHAR_LIST:
			return bool(value_.index_!=0);
		case STRING:
			return false ;
	} ;
	return false ;
} ;

void Variable::SetString(const char *string,bool notify) {
	NAssert(string);
	switch(type_) {
		case FLOAT:
			value_.float_=float(atof(string));
			break ;
		case INT:
			value_.int_=atoi(string);
			break ;
		case BOOL:
			value_.bool_=(!strcmp("false",string)?false:true) ;
			break ;
		case STRING:
			stringValue_=string ;
			break ;
		case CHAR_LIST:
			value_.index_=-1 ;
			for (int i=0;i<listSize_;i++) {
				if (list_.char_[i]) {
                     const char *d=list_.char_[i] ;
                     const char *s=string ;
                     while (*s!=0) {
                           if (tolower(*s++)!=tolower(*d++)) {
                              break ;
                           }
                     }
                     if (*s==0) {
						value_.index_=i ;
						break ;
                     }
				}
			} ;
			break ;
	} ;
	if (notify) {
    	onChange();
    }
} ;

const char *Variable::GetString() {
	string_[0]=0 ;
	switch(type_) {
		case FLOAT:
			sprintf(string_,"%f",value_.float_);
			break ;
		case INT:
			sprintf(string_,"%d",value_.int_);
			break ;
		case BOOL:
			strcpy(string_,value_.bool_?"true":"false") ;
			break ;
		case CHAR_LIST:
            if ((value_.index_<0)||(value_.index_>=listSize_)) {
                return "(null)" ;
            } else {
    			return list_.char_[value_.index_] ;
            }
			break ;
		case STRING:
			return stringValue_.c_str() ;
	} ;
	return string_ ;
};

void Variable::CopyFrom(Variable &other) {
	type_=other.type_ ;
	value_=other.value_ ;
	list_=other.list_ ;
	listSize_=other.listSize_ ;
	onChange();
}

char **Variable::GetListPointer() {
	NAssert(type_==CHAR_LIST) ;
	return list_.char_ ;
} ;

int Variable::GetListSize() {
	NAssert(type_==CHAR_LIST) ;
	return listSize_ ;
} ;

void Variable::Reset() {

	switch(type_) {

		case FLOAT:
			value_.float_=defaultValue_.float_ ;
			break ;
		case INT:
			value_.int_=defaultValue_.int_ ;
			break ;
		case BOOL:
			value_.bool_=defaultValue_.bool_ ;
			break ;
		case CHAR_LIST:
			value_.index_=defaultValue_.index_ ;
			break ;
		case STRING:
			stringValue_=stringDefaultValue_ ;
			break;
	}
}
