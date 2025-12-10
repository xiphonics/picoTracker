/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _CONTROL_NODE_H_
#define _CONTROL_NODE_H_

#include "Foundation/Observable.h"
#include "Foundation/T_SimpleList.h"
#include "Services/Controllers/Channel.h"
#include "Services/Controllers/ControllerService.h"
#include "config/StringLimits.h"
#include "Externals/etl/include/etl/string.h"

enum ControlNodeType { CNT_NODE, CNT_ASSIGNABLE };

class ControlNode : protected T_SimpleList<ControlNode> {
public:
  ControlNode(const char *name, ControlNode *parent);
  virtual ~ControlNode();

  virtual ControlNode *FindChild(const etl::string<STRING_CONTROL_PATH_MAX> &url,
                                 bool create = false);
  virtual ControlNode *FindChild(const char *url, bool create = false);
  virtual etl::string<STRING_CONTROL_PATH_MAX> GetPath();
  ControlNodeType GetType() { return type_; };
  virtual void Trigger();

  void Dump(int level);

protected:
  etl::string<STRING_CONTROL_NAME_MAX> name_;
  ControlNode *parent_;
  ControlNodeType type_;
};

// These nodes can also be assigned a channel
// They implement observable so that a we can watch a change in
// channel assignment

class AssignableControlNode : public ControlNode,
                              public Channel,
                              public I_Observer {
public:
  AssignableControlNode(const char *name, ControlNode *parent);
  virtual ~AssignableControlNode();
  bool SetSourceChannel(Channel *);
  Channel *GetSourceChannel();

protected:
  virtual void Update(Observable &o, I_ObservableData *d);

public:
  Channel *channel_;
};
#endif
