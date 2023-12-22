#ifndef _VIEW_EVENT_H_
#define _VIEW_EVENT_H_

#include "Foundation/Observable.h"

enum ViewEventType {
  VET_SWITCH_VIEW,
  VET_PLAYER_POSITION_UPDATE,
  VET_SAVEAS_PROJECT,
  VET_LIST_SELECT,
  VET_QUIT_PROJECT,
  VET_UPDATE,
  VET_QUIT_APP
};

class ViewEvent : public I_ObservableData {
public:
  ViewEvent(ViewEventType type, void *data = 0);
  ViewEventType GetType();
  void *GetData();

private:
  ViewEventType type_;
  void *data_;
};

#endif
