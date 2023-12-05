
#include "ControlRoom.h"
#include "Services/Controllers/ControllerService.h"
#include "Services/Controllers/MultiChannelAdapter.h"
#include "System/Console/Trace.h"
#include <string>

ControlRoom::ControlRoom() : ControlNode("", 0){};

ControlRoom::~ControlRoom(){};

bool ControlRoom::Init() { return true; };

void ControlRoom::Close() { Empty(); };

AssignableControlNode *ControlRoom::GetControlNode(const std::string url) {

  // Look if the node exists already

  ControlNode *existing = FindChild(url);
  if (existing) {
    return (existing->GetType() == CNT_ASSIGNABLE)
               ? (AssignableControlNode *)existing
               : 0;
  }

  // We need to create it.
  std::string::size_type pos = url.find_last_of("/");
  std::string suburl = url.substr(0, pos);
  ControlNode *parent = this->FindChild(suburl, true);
  AssignableControlNode *newNode =
      new AssignableControlNode(url.substr(pos + 1).c_str(), parent);
  return newNode;
};

void ControlRoom::Dump() { ControlNode::Dump(0); };

bool ControlRoom::Attach(const char *nodeUrl, const char *controllerUrl) {

  if (controllerUrl[0] == 0)
    return true;

  ControlNode *existing = FindChild(nodeUrl);
  if ((!existing) || (existing->GetType() != CNT_ASSIGNABLE)) {
    Trace::Error("Trying to map unknown node %s", nodeUrl);
    return false;
  }

  AssignableControlNode *acn = (AssignableControlNode *)existing;

  MultiChannelAdapter *mca = (MultiChannelAdapter *)acn->GetSourceChannel();
  if (!mca) {
    std::string name = acn->GetName();
    name += "-adapter";
    mca = new MultiChannelAdapter(name.c_str());
    acn->SetSourceChannel(mca);
  }

  Channel *channel =
      ControllerService::GetInstance()->GetChannel(controllerUrl);

  if (channel) {
    mca->AddChannel(*channel);
    Trace::Log("MAPPING", "Attached %s to %s", nodeUrl, controllerUrl);
  } else {
    Trace::Debug("Failed to attach %s to %s", nodeUrl, controllerUrl);
  };
  return true;
};
