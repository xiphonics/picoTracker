
#include "ServiceRegistry.h"

void ServiceRegistry::Register(Service *s) { services_.Insert(s); };

void ServiceRegistry::Register(SubService *s) {
  for (services_.Begin(); !services_.IsDone(); services_.Next()) {
    Service &current = services_.CurrentItem();
    if (current.GetFourCC() == s->GetFourCC()) {
      current.Register(s);
    };
  };
};

void ServiceRegistry::Unregister(SubService *s) {
  for (services_.Begin(); !services_.IsDone(); services_.Next()) {
    Service &current = services_.CurrentItem();
    if (current.GetFourCC() == s->GetFourCC()) {
      current.Unregister(s);
    };
  };
};
