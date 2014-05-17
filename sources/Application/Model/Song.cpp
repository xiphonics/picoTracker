#include "Song.h"
#include "System/io/Status.h"
#include "Application/Utils/HexBuffers.h"
#include "Application/Instruments/CommandList.h"
#include "Table.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


Song::Song():Persistent("SONG") {

	data_=(unsigned char *)SYS_MALLOC(SONG_CHANNEL_COUNT*SONG_ROW_COUNT) ;
	memset(data_,0xFF,SONG_CHANNEL_COUNT*SONG_ROW_COUNT) ;
	
	chain_=new Chain() ;   // Allocate chain datas
	phrase_=new Phrase() ; // Allocate phrase datas
} ;

Song::~Song() {
	if (data_!=NULL) SYS_FREE (data_) ;
	delete chain_ ;
	delete phrase_ ;
} ;

void Song::SaveContent(TiXmlNode *node) {
	for (int i=0; i<PHRASE_COUNT*16; i++)
	{
		phrase_->param1_[i] = Swap16(phrase_->param1_[i]);
		phrase_->param2_[i] = Swap16(phrase_->param2_[i]);
	}	
	saveHexBuffer(node,"SONG",data_,SONG_ROW_COUNT*SONG_CHANNEL_COUNT) ;
	saveHexBuffer(node,"CHAINS",chain_->data_,CHAIN_COUNT*16) ;
	saveHexBuffer(node,"TRANSPOSES",chain_->transpose_,CHAIN_COUNT*16) ;
	saveHexBuffer(node,"NOTES",phrase_->note_,PHRASE_COUNT*16) ;
	saveHexBuffer(node,"INSTRUMENTS",phrase_->instr_,PHRASE_COUNT*16) ;
	saveHexBuffer(node,"COMMAND1",phrase_->cmd1_,PHRASE_COUNT*16) ;
	saveHexBuffer(node,"PARAM1",phrase_->param1_,PHRASE_COUNT*16) ;
	saveHexBuffer(node,"COMMAND2",phrase_->cmd2_,PHRASE_COUNT*16) ;
	saveHexBuffer(node,"PARAM2",phrase_->param2_,PHRASE_COUNT*16) ;

} ;

void Song::RestoreContent(TiXmlElement *element) {

	TiXmlElement *current=element->FirstChildElement() ;
	while(current) {
		const char *value=current->Value() ;
		if (!strcmp("SONG",value)) {
			restoreHexBuffer(current,data_) ;
		} ;
		if (!strcmp("CHAINS",value)) {
			restoreHexBuffer(current,chain_->data_) ;
		} ;
		if (!strcmp("TRANSPOSES",value)) {
			restoreHexBuffer(current,chain_->transpose_) ;
		} ;
		if (!strcmp("NOTES",value)) {
			restoreHexBuffer(current,phrase_->note_) ;
		} ;
		if (!strcmp("INSTRUMENTS",value)) {
			restoreHexBuffer(current,phrase_->instr_) ;
		} ;
		if (!strcmp("COMMAND1",value)) {
			restoreHexBuffer(current,(uchar *)phrase_->cmd1_) ;
		} ;
		if (!strcmp("PARAM1",value)) {
			restoreHexBuffer(current,(uchar *)phrase_->param1_) ;
		} ;
		if (!strcmp("COMMAND2",value)) {
			restoreHexBuffer(current,(uchar *)phrase_->cmd2_) ;
		} ;
		if (!strcmp("PARAM2",value)) {
			restoreHexBuffer(current,(uchar *)phrase_->param2_) ;
		} ;
		
		
		for (int i=0; i<PHRASE_COUNT*16; i++)
		{
			phrase_->param1_[i] = Swap16(phrase_->param1_[i]);
			phrase_->param2_[i] = Swap16(phrase_->param2_[i]);
		}
		
		current=current->NextSiblingElement() ;
	} ;
	
	Status::Set("Restoring allocation") ;

	// Restore chain & phrase allocation table

	unsigned char *data=data_ ;
	for (int i=0;i<256*SONG_CHANNEL_COUNT;i++) {
		if (*data!=0xFF) {
			if (*data<0x80) {
				chain_->SetUsed(*data) ;
			}	
		}
	 	data++ ;
	}

    data=chain_->data_ ;

	for (int i=0;i<CHAIN_COUNT;i++) {
        for (int j=0;j<16;j++) {
            if (*data!=0xFF) {
                chain_->SetUsed(i) ;
                phrase_->SetUsed(*data) ;
			}
            data++ ;
        } ;
    }

    data=phrase_->note_ ;

	FourCC *table1=phrase_->cmd1_ ;
	FourCC *table2=phrase_->cmd2_ ;

	ushort *param1=phrase_->param1_ ;
	ushort *param2=phrase_->param2_ ;

	TableHolder *th=TableHolder::GetInstance() ;

	for (int i=0;i<PHRASE_COUNT;i++) {
        for (int j=0;j<16;j++) {
            if (*data!=0xFF) {
                phrase_->SetUsed(i) ;
            }
			if (*table1==I_CMD_TABL) {
				*param1&=0x7F ;
				th->SetUsed((*param1)) ;
			} ;
			if (*table2==I_CMD_TABL) {
				*param2&=0x7F ;
				th->SetUsed((*param2)) ;
			} ;
			table1++ ;
			table2++ ;
			param1++ ;
			param2++ ;
            data++ ;
    	} ;
    }
};
