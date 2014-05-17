
#ifndef _CONTROL_NODE_H_
#define _CONTROL_NODE_H_

#include "Foundation/T_SimpleList.h"
#include "Services/Controllers/Channel.h"
#include "Foundation/Observable.h"
#include "Services/Controllers/ControllerService.h"

enum ControlNodeType {
	CNT_NODE,
	CNT_ASSIGNABLE
} ;

class ControlNode:protected T_SimpleList<ControlNode> {
public:
	ControlNode(const char *name,ControlNode *parent) ;
	virtual ~ControlNode();

	virtual ControlNode *FindChild(const std::string &url,bool create=false) ;
	virtual std::string GetPath() ;
	ControlNodeType GetType() {return type_ ;} ;
	virtual void Trigger() ;

	void Dump(int level) ;
protected:
	std::string name_ ;
	ControlNode *parent_ ;
	ControlNodeType type_ ;
} ;

// These nodes can also be assigned a channel
// They implement observable so that a we can watch a change in
// channel assignment

class AssignableControlNode: public ControlNode,public Channel,public I_Observer {
public:
	AssignableControlNode(const char *name,ControlNode *parent) ;
	virtual ~AssignableControlNode() ;
	bool SetSourceChannel(Channel *) ;
	Channel *GetSourceChannel() ;
protected:
	virtual void Update(Observable &o,I_ObservableData *d) ;
public:
	Channel *channel_ ;
} ;
#endif
