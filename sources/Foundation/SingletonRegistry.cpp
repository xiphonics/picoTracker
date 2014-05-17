
#include "SingletonRegistry.h"

SingletonRegistry _instance ;

SingletonRegistry::SingletonRegistry():T_SimpleList<I_Singleton>(true) {
} ;

SingletonRegistry::~SingletonRegistry() {
} ;

SingletonRegistry *SingletonRegistry::GetInstance() {
	return &_instance ;
} ;
