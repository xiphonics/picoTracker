/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "ControlNode.h"
#include "Services/Controllers/ControllerService.h"
#include "System/Console/Trace.h"
#include <string.h>

ControlNode::ControlNode(const char *name, ControlNode *parent)
    : T_SimpleList<ControlNode>(true), type_(CNT_NODE) {
  name_ = name;
  parent_ = parent;
  if (parent)
    parent->Insert(this);
};

ControlNode::~ControlNode() { Empty(); };

ControlNode *ControlNode::FindChild(
    const etl::string<STRING_CONTROL_PATH_MAX> &url, bool create) {
  if (url.length() == 0) {
    return NULL;
  }

  etl::string<STRING_CONTROL_PATH_MAX> suburl = url;
  if (parent_ == 0) {
    suburl = url.substr(1); // Get rid of the first slash
  }
  etl::string<STRING_CONTROL_PATH_MAX>::size_type pos = suburl.find("/", 0);
  etl::string<STRING_CONTROL_PATH_MAX> node = suburl;
  if (pos != etl::string<STRING_CONTROL_PATH_MAX>::npos) {
    node = suburl.substr(0, pos);
  }

  for (Begin(); !IsDone(); Next()) {
    ControlNode &current = CurrentItem();
    if (!strcmp(node.c_str(), current.name_.c_str())) {
      if (pos == etl::string<STRING_CONTROL_PATH_MAX>::npos) {
        return &current;
      };
      return current.FindChild(suburl.substr(pos + 1), create);
    };
  };

  if (create) {
    ControlNode *parent = this;
    ControlNode *newnode = 0;
    while (suburl.size() != 0) {
      etl::string<STRING_CONTROL_PATH_MAX>::size_type pos =
          suburl.find("/", 0);
      if (pos != etl::string<STRING_CONTROL_PATH_MAX>::npos) {
        node = suburl.substr(0, pos);
        suburl = suburl.substr(pos + 1);
      } else {
        node = suburl;
        suburl = "";
      }
      newnode = new ControlNode(node.c_str(), parent);
      parent = newnode;
    }
    return newnode;
  };
  return 0;
};

ControlNode *ControlNode::FindChild(const char *url, bool create) {
  etl::string<STRING_CONTROL_PATH_MAX> asString(url);
  return FindChild(asString, create);
}

etl::string<STRING_CONTROL_PATH_MAX> ControlNode::GetPath() {
  etl::string<STRING_CONTROL_PATH_MAX> path;
  if (parent_) {
    path = parent_->GetPath();
  };
  path += "/";
  path += name_;
  return path;
};

void ControlNode::Trigger() {
  for (Begin(); !IsDone(); Next()) {
    ControlNode &current = CurrentItem();
    current.Trigger();
  }
};

void ControlNode::Dump(int level) {
  char levelst[30];
  strcpy(levelst, "                  ");
  levelst[level] = 0;
  Trace::Debug("%s %s", levelst, name_.c_str());
  for (Begin(); !IsDone(); Next()) {
    ControlNode &current = CurrentItem();
    current.Dump(level + 1);
  };
};
///////////////////////////////////////////

AssignableControlNode::AssignableControlNode(const char *name,
                                             ControlNode *parent)
    : ControlNode(name, parent), Channel(name) {
  type_ = CNT_ASSIGNABLE;
  channel_ = 0;
};

AssignableControlNode::~AssignableControlNode() {
  if (channel_) {
    channel_->RemoveObserver(*this);
  };
};

bool AssignableControlNode::SetSourceChannel(Channel *channel) {
  if (channel_) {
    channel_->RemoveObserver(*this);
  };
  channel_ = channel;
  if (channel_) {
    SetValue(channel_->GetValue());
    channel_->AddObserver(*this);
  }
  return (channel_ != 0);
};

Channel *AssignableControlNode::GetSourceChannel() { return channel_; };

void AssignableControlNode::Update(Observable &o, I_ObservableData *d) {
  Channel &channel = (Channel &)o;
  SetValue(channel.GetValue());
  NotifyObservers();
};
