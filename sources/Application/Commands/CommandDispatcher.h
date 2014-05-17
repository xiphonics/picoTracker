#ifndef _COMMAND_DISPATCHER_H_
#define _COMMAND_DISPATCHER_H_

#include "Foundation/T_Singleton.h"
#include "Foundation/Observable.h"
#include "Foundation/T_SimpleList.h"
#include "Foundation/Types/Types.h"
#include "Services/Controllers/ControlNode.h"
#include "NodeList.h"

class CommandExecuter {
public:
	CommandExecuter() {} ;
	virtual ~CommandExecuter() {} ;
	virtual void Execute(FourCC id,float value)=0 ;
} ;

class CommandTrigger:I_Observer {
public:
	CommandTrigger(FourCC id,CommandExecuter &executer) ;
	~CommandTrigger() ;
	void Attach(AssignableControlNode &node) ;
	void Detach() ;
protected:
	virtual void Update(Observable &o,I_ObservableData *d) ;	
private:
	AssignableControlNode *node_ ;
	int id_ ;
	CommandExecuter &executer_ ;
} ;

class CommandDispatcher:public T_Singleton<CommandDispatcher>,T_SimpleList<CommandTrigger>,CommandExecuter {
public:
	CommandDispatcher() ;
	virtual ~CommandDispatcher() ;
	bool Init() ;
	void Close() ;

	virtual void Execute(FourCC id,float value) ;

protected:
	void mapTrigger(FourCC trigger,const char *nodeUrl,CommandExecuter &executer) ; 
} ;
#endif
