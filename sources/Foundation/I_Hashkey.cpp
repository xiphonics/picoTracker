
#include "I_Hashkey.h"

bool I_Hashkey::operator==(const I_Hashkey &other) {
	return this->Equals(other) ;
}

bool I_Hashkey::operator!=(const I_Hashkey &other) {
	return !this->Equals(other) ;
}
