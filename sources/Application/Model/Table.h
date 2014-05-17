#ifndef _TABLE_H_
#define _TABLE_H_

#include "Foundation/Types/Types.h"
#include "Foundation/T_Singleton.h"
#include "Application/Persistency/Persistent.h"

#define TABLE_COUNT 0x80
#define TABLE_STEPS 16

#define NO_MORE_TABLE TABLE_COUNT+10


class Table {
public:
	Table() ;
	void Reset() ;
	bool IsEmpty() ;
	void Copy(const Table &other) ;
public:
	FourCC cmd1_[TABLE_STEPS] ;
	ushort param1_[TABLE_STEPS] ;
	FourCC cmd2_[TABLE_STEPS] ;
	ushort param2_[TABLE_STEPS] ;
	FourCC cmd3_[TABLE_STEPS] ;
	ushort param3_[TABLE_STEPS] ;
} ; 

class TableHolder: public T_Singleton<TableHolder>,Persistent  {
public:
	TableHolder() ;
	void Reset() ;
	Table &GetTable(int table) ;
	void SetUsed(int table) ;	
	int GetNext() ;
	int Clone(int table) ;
	virtual void SaveContent(TiXmlNode *node) ;
	virtual void RestoreContent(TiXmlElement *element);

private:
	Table table_[TABLE_COUNT] ;
	bool allocation_[TABLE_COUNT] ;
} ;

#endif
