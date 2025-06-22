/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _SUBSERVICE_H_
#define _SUBSERVICE_H_

class SubService {
public:
  SubService(int fourCC);
  virtual ~SubService();
  int GetFourCC() { return fourCC_; };

private:
  int fourCC_;
};
#endif
