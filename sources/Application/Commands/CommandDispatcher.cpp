#include "CommandDispatcher.h"
#include "ApplicationCommandDispatcher.h"
#include "EventDispatcher.h"

#include "Application/Controllers/ControlRoom.h"
#include "Application/Mixer/MixerService.h"
#include "System/Console/Trace.h"

CommandTrigger::CommandTrigger(FourCC id, CommandExecuter &executer)
    : node_(0), id_(id), executer_(executer){};

CommandTrigger::~CommandTrigger(){};

void CommandTrigger::Attach(AssignableControlNode &node) {
  Detach();
  node_ = &node;
  node_->AddObserver(*this);
};

void CommandTrigger::Detach() {
  if (node_) {
    node_->RemoveObserver(*this);
    node_ = 0;
  }
};

void CommandTrigger::Update(Observable &o, I_ObservableData *d) {
  executer_.Execute(id_, node_->GetValue());
};

CommandDispatcher::CommandDispatcher(){};

CommandDispatcher::~CommandDispatcher(){};

bool CommandDispatcher::Init() {

  MixerService *ms = MixerService::GetInstance();
  mapTrigger(FourCC::TrigVolumeIncrease, URL_VOLUME_INCREASE, *ms);
  mapTrigger(FourCC::TrigVolumeDecrease, URL_VOLUME_DECREASE, *ms);

  EventDispatcher *ed = EventDispatcher::GetInstance();
  mapTrigger(FourCC::TrigEventEnter, URL_EVENT_A, *ed);
  mapTrigger(FourCC::TrigEventEdit, URL_EVENT_B, *ed);
  mapTrigger(FourCC::TrigEventUp, URL_EVENT_UP, *ed);
  mapTrigger(FourCC::TrigEventDown, URL_EVENT_DOWN, *ed);
  mapTrigger(FourCC::TrigEventLeft, URL_EVENT_LEFT, *ed);
  mapTrigger(FourCC::TrigEventRight, URL_EVENT_RIGHT, *ed);
  mapTrigger(FourCC::TrigEventAlt, URL_EVENT_LSHOULDER, *ed);
  mapTrigger(FourCC::TrigEventNav, URL_EVENT_RSHOULDER, *ed);
  mapTrigger(FourCC::TrigEventPlay, URL_EVENT_START, *ed);

  ApplicationCommandDispatcher *acd =
      ApplicationCommandDispatcher::GetInstance();
  mapTrigger(FourCC::TrigTempoTap, URL_TEMPO_TAP, *acd);
  mapTrigger(FourCC::TrigSeqQueueRow, URL_QUEUE_ROW, *acd);

  //	ControlRoom::GetInstance()->Dump() ;
  return true;
};

void CommandDispatcher::Close(){};

void CommandDispatcher::mapTrigger(FourCC trigger, const char *nodeUrl,
                                   CommandExecuter &executer) {

  ControlRoom *cr = ControlRoom::GetInstance();
  AssignableControlNode *acn = cr->GetControlNode(nodeUrl);
  if (acn) {
    CommandTrigger *ct = new CommandTrigger(trigger, executer);
    Insert(ct);
    ct->Attach(*acn);
  } else {
    Trace::Error("Failed to find node %s", nodeUrl);
  }
};

void CommandDispatcher::Execute(FourCC id, float value) {
  Trace::Debug("Got called for %d(%f)", id, value);
}
