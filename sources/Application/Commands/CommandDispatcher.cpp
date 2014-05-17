#include "CommandDispatcher.h"
#include "EventDispatcher.h"
#include "ApplicationCommandDispatcher.h"

#include "Application/Controllers/ControlRoom.h"
#include "Application/Mixer/MixerService.h"
#include "System/Console/Trace.h"

CommandTrigger::CommandTrigger(FourCC id,CommandExecuter &executer):
    node_(0),
	id_(id),
	executer_(executer)
{
} ;

CommandTrigger::~CommandTrigger() {
} ;

void CommandTrigger::Attach(AssignableControlNode &node) {
	Detach() ;
	node_=&node ;
	node_->AddObserver(*this) ;
} ;

void CommandTrigger::Detach() {
	if (node_) {
		node_->RemoveObserver(*this) ;
		node_=0 ;
	}
} ;

void CommandTrigger::Update(Observable &o,I_ObservableData *d) {
	executer_.Execute(id_,node_->GetValue()) ;
} ;

CommandDispatcher::CommandDispatcher() {
} ;

CommandDispatcher::~CommandDispatcher() {
} ;

bool CommandDispatcher::Init() {

    MixerService* ms=MixerService::GetInstance() ;
	mapTrigger(TRIG_VOLUME_INCREASE,URL_VOLUME_INCREASE,*ms) ;
	mapTrigger(TRIG_VOLUME_DECREASE,URL_VOLUME_DECREASE,*ms) ;

	EventDispatcher *ed=EventDispatcher::GetInstance() ;
	mapTrigger(TRIG_EVENT_A,URL_EVENT_A,*ed) ;
	mapTrigger(TRIG_EVENT_B,URL_EVENT_B,*ed) ;
	mapTrigger(TRIG_EVENT_UP,URL_EVENT_UP,*ed) ;
	mapTrigger(TRIG_EVENT_DOWN,URL_EVENT_DOWN,*ed) ;
	mapTrigger(TRIG_EVENT_LEFT,URL_EVENT_LEFT,*ed) ;
	mapTrigger(TRIG_EVENT_RIGHT,URL_EVENT_RIGHT,*ed) ;
	mapTrigger(TRIG_EVENT_LSHOULDER,URL_EVENT_LSHOULDER,*ed) ;
	mapTrigger(TRIG_EVENT_RSHOULDER,URL_EVENT_RSHOULDER,*ed) ;
	mapTrigger(TRIG_EVENT_START,URL_EVENT_START,*ed) ;

	ApplicationCommandDispatcher *acd=ApplicationCommandDispatcher::GetInstance() ;
	mapTrigger(TRIG_TEMPO_TAP,URL_TEMPO_TAP,*acd) ;
	mapTrigger(TRIG_SEQ_QUEUE_ROW,URL_QUEUE_ROW,*acd) ;

	//	ControlRoom::GetInstance()->Dump() ;
	return true ;
}  ;

void CommandDispatcher::Close() {
} ;

void CommandDispatcher::mapTrigger(FourCC trigger,const char *nodeUrl,CommandExecuter &executer) {

	ControlRoom *cr=ControlRoom::GetInstance() ;
	AssignableControlNode *acn=cr->GetControlNode(nodeUrl) ; 
	if (acn) {
		CommandTrigger *ct=new CommandTrigger(trigger,executer) ;
		Insert(ct) ;
		ct->Attach(*acn) ;
	} else {
		Trace::Error("Failed to find node %s",nodeUrl) ;
	}
} ;

void CommandDispatcher::Execute(FourCC id,float value) {
	Trace::Debug("Got called for %d(%f)",id,value) ;
}
