
#include "MultiChannelAdapter.h"
#include "Services/Controllers/ControllerService.h"

MultiChannelAdapter::MultiChannelAdapter(const char *name, bool owner)
    : T_SimpleList<Channel>(owner), Channel(name){};

MultiChannelAdapter::~MultiChannelAdapter() {
  for (Begin(); !IsDone(); Next()) {
    Channel &current = CurrentItem();
    current.RemoveObserver(*this);
  };
};

bool MultiChannelAdapter::AddChannel(const char *path) {
  Channel *channel = ControllerService::GetInstance()->GetChannel(path);
  if (channel) {
    Insert(channel);
    channel->AddObserver(*this);
  };
  return (channel != 0);
};

void MultiChannelAdapter::AddChannel(Channel &channel) {
  Insert(channel);
  channel.AddObserver(*this);
};

void MultiChannelAdapter::Update(Observable &o, I_ObservableData *d) {
  Channel &source = (Channel &)o;

  value_ = source.GetValue();
  for (Begin(); !IsDone(); Next()) {
    Channel &current = CurrentItem();
    if (&current != &source) {
      current.SetValue(value_, false);
    };
  };
  SetChanged();
  NotifyObservers();
};

void MultiChannelAdapter::SetValue(float value, bool notify) {

  value_ = value;
  for (Begin(); !IsDone(); Next()) {
    Channel &current = CurrentItem();
    current.SetValue(value, false);
  };
  if (notify) {
    SetChanged();
    NotifyObservers();
  };
}
