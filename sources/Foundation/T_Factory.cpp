
template <class Item> Item* T_Factory<Item>::instance_=0 ;

template <class Item>
void T_Factory<Item>::Install(Item *instance) {
	instance_=instance ;
}
template <class Item>
Item* T_Factory<Item>::GetInstance() {
	return instance_ ;
}
