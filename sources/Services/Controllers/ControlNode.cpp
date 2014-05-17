#include "Services/Controllers/ControllerService.h"
#include "ControlNode.h"
#include "System/Console/Trace.h"
#include <string.h>

using namespace std;

ControlNode::ControlNode(const char *name,ControlNode *parent):T_SimpleList<ControlNode>(true),type_(CNT_NODE) {
	name_=name ;
	parent_=parent;
	if (parent) parent->Insert(this) ;
} ;
	
ControlNode::~ControlNode() {
	Empty() ;
} ;

ControlNode *ControlNode::FindChild(const std::string &url,bool create) {
	if (url.length() == 0) {
		return NULL;
	}

	std::string suburl=url ; 
	if (parent_==0) {
		suburl=url.substr(1) ; // Get rid of the first slash
	}
	string::size_type pos=suburl.find("/",0) ;
	std::string node=suburl ;
	if (pos != string::npos) {
		node=suburl.substr(0,pos) ;
	}

	IteratorPtr<ControlNode>it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		ControlNode &current=it->CurrentItem() ;
		if (!strcmp(node.c_str(),current.name_.c_str())) {
			if ( pos == string::npos) {
				return &current ;
			};
			return current.FindChild(suburl.substr(pos+1),create) ;
		} ;
	} ;


	if (create) {
		ControlNode *parent=this ;
		ControlNode *newnode=0 ;
		while (suburl.size()!=0) {
			string::size_type pos=suburl.find("/",0) ;
			if (pos != string::npos) {
				node=suburl.substr(0,pos) ;
				suburl=suburl.substr(pos+1) ;
			} else {
				node=suburl ;
				suburl="" ;
			}
			newnode=new ControlNode(node.c_str(),parent) ;
			parent=newnode ;
		}
		return newnode ;
	} ;
	return 0 ;
} ;

std::string ControlNode::GetPath() {
	std::string path ;
 	if (parent_) {
		path=parent_->GetPath() ;
	} ;
	path+="/" ;
	path+=name_ ;
	return path ;
} ;

void ControlNode::Trigger() {
	IteratorPtr<ControlNode>it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		ControlNode &current=it->CurrentItem() ;
		current.Trigger() ;
	}
} ;

void ControlNode::Dump(int level) {
	char levelst[30] ;
	strcpy(levelst,"                  ") ;
	levelst[level]=0 ;
	Trace::Debug("%s %s",levelst,name_.c_str()) ;
	IteratorPtr<ControlNode>it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		ControlNode &current=it->CurrentItem() ;
		current.Dump(level+1) ;
	} ;
} ;
///////////////////////////////////////////

AssignableControlNode::AssignableControlNode(const char *name,ControlNode *parent):ControlNode(name,parent),Channel(name) {
	type_=CNT_ASSIGNABLE ;
	channel_=0 ;
} ;

AssignableControlNode::~AssignableControlNode() {
	if (channel_) {
		channel_->RemoveObserver(*this) ;
	} ;
} ;

bool AssignableControlNode::SetSourceChannel(Channel *channel) {
	if (channel_) {
		channel_->RemoveObserver(*this) ;
	} ;
	channel_=channel ;
	if (channel_) {
		SetValue(channel_->GetValue()) ;
		channel_->AddObserver(*this) ;
	} 
	return (channel_!=0) ;
} ;

Channel *AssignableControlNode::GetSourceChannel() {
	return channel_ ;
} ;

void AssignableControlNode::Update(Observable &o,I_ObservableData *d) {
	Channel &channel=(Channel &)o ;
	SetValue(channel.GetValue()) ;
	NotifyObservers() ;
} ;
