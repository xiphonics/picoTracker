/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

template <class Item> T_Singleton<Item>::T_Singleton() {}

template <class Item> T_Singleton<Item>::~T_Singleton() {}

template <class Item> Item *T_Singleton<Item>::GetInstance() {
  if (!etl::singleton<Item>::is_valid()) {
    etl::singleton<Item>::create();
  }
  return &etl::singleton<Item>::instance();
}

template <class Item>
template <typename... TArgs>
Item *T_Singleton<Item>::Create(TArgs &&...args) {
  etl::singleton<Item>::create(etl::forward<TArgs>(args)...);
  return &etl::singleton<Item>::instance();
}
