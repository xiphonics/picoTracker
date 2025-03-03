
template <class Item> Item *T_Singleton<Item>::instance_ = 0;

template <class Item> T_Singleton<Item>::T_Singleton() {}

template <class Item> T_Singleton<Item>::~T_Singleton() {}

template <class Item> Item *T_Singleton<Item>::GetInstance() {
  if (instance_ == 0) {
    instance_ = new Item;
  };
  return instance_;
}
