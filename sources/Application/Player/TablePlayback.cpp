#include "TablePlayback.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Instruments/I_Instrument.h"

TablePlayback TablePlayback::playback_[SONG_CHANNEL_COUNT] ;

void TableSaveState::Reset() {
		position_[0]=0 ;
		position_[1]=0 ;
		position_[2]=0 ;
		for (int i=0;i<TABLE_STEPS;i++) {
			hopCount_[i][0]=0 ;
			hopCount_[i][1]=0 ;
			hopCount_[i][2]=0 ;
		};
} ;

void TablePlayback::Reset() {
	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		playback_[i].Init(i) ;
	}
} ;

TablePlayback &TablePlayback::GetTablePlayback(int channel) {
	NAssert((channel>=0)&&(channel<SONG_CHANNEL_COUNT)) ;
	return playback_[channel] ;
}

void TablePlayback::Init(int channel) {
	channel_=channel ;
	table_=0 ;
	position_[0]=0 ;
	position_[1]=0 ;
	position_[2]=0 ;

	hopped_[0]=false ;
	hopped_[1]=false ;
	hopped_[2]=false ;

	for (int i=0;i<TABLE_STEPS;i++) {
		hopCount_[i][0]=0 ;
		hopCount_[i][1]=0 ;
		hopCount_[i][2]=0 ;
	};
	instrument_=0 ;
	groove_.groove_=-1 ;
	groove_.position_=0 ;
	groove_.ticks_=0 ;

	automated_=false ;
}

void TablePlayback::Start(I_Instrument *i,Table &table,bool automated) {
	if ((!automated)||(automated_!=automated)||(i!=instrument_)||(table_==0)) {
		instrument_=i ;
		position_[0]=0 ;
		position_[1]=0 ;
		position_[2]=0 ;

		hopped_[0]=false ;
		hopped_[1]=false ;
		hopped_[2]=false ;

		for (int i=0;i<TABLE_STEPS;i++) {
			hopCount_[i][0]=0 ;
			hopCount_[i][1]=0 ;
			hopCount_[i][2]=0 ;
		};
		groove_.groove_=-1 ;
		groove_.position_=0 ;
		groove_.ticks_=0 ;

		automated_=automated ;
	}
	table_=&table;
}

void TablePlayback::Stop() {
	table_=0 ;
	position_[0]=0 ;
	position_[1]=0 ;
	position_[2]=0 ;

	hopped_[0]=false ;
	hopped_[1]=false ;
	hopped_[2]=false ;
} ;

int TablePlayback::GetPlaybackPosition(int i) {
	return previous_[i] ;
}

Table *TablePlayback::GetTable() {
	return table_ ;
} ;

bool TablePlayback::GetAutomation() {
	return automated_ ;
} ;

bool TablePlayback::ProcessLocalCommand(int row,FourCC *commandList,ushort *paramList,TablePlayerChange &tpc) {

	bool hopped=false ;

	FourCC command=commandList[position_[row]] ;
	ushort param=paramList[position_[row]] ;

    // First process any positional command

	switch(command) {
		case I_CMD_HOP:
		{
			int count=param>>8  ;
			if (hopCount_[position_[row]][row]==0) {
				hopCount_[position_[row]][row]=count ;
			} else {
				hopCount_[position_[row]][row]-- ;
			};
			if ((hopCount_[position_[row]][row]!=0)||(count==0)) {
				position_[row]=param&0xF ;
				hopped=true ;
			} else {
				position_[row]=(position_[row]+1)%16 ;
				hopped=true ;
			};
			break ;
		}
	}

	// Update values if needed

	if (hopped) {
		command=commandList[position_[row]] ;
		param=paramList[position_[row]] ;
	}

	// Now process local command on possibly hopped row

	switch(command) {
		case I_CMD_KILL:
			tpc.timeToLive_=(param&0xFF)+1 ;
			break ;
		case I_CMD_IRTG:
			tpc.instrRetrigger_=(param&0xFF) ;
			break ;
		case I_CMD_GROV:
			param=param&0x1F ;
			groove_.groove_=(unsigned char)param ;
			groove_.position_=0 ;
			groove_.ticks_=0 ;
			break ;
	}
	return hopped ;
}

void TablePlayback::ProcessStep(TablePlayerChange &tpc) {	

	Groove *gs=Groove::GetInstance();

	if (table_!=0) {
        if (instrument_) {

			// See if groove tells us we need to process a step

			if (groove_.ticks_==0) {

				// If automated, restore state

				if (automated_) {
					TableSaveState state ;
					instrument_->GetTableState(state) ;
					memcpy(hopCount_,state.hopCount_,sizeof(uchar)*TABLE_STEPS*2) ;
					memcpy(position_,state.position_,sizeof(int)*2) ;
				}

				// try local processing for if it changes current table or position

				hopped_[0]=ProcessLocalCommand(0,table_->cmd1_,table_->param1_,tpc) ;
				hopped_[1]=ProcessLocalCommand(1,table_->cmd2_,table_->param2_,tpc) ;
				hopped_[2]=ProcessLocalCommand(2,table_->cmd3_,table_->param3_,tpc) ;

				instrument_->ProcessCommand(channel_,table_->cmd1_[position_[0]],table_->param1_[position_[0]]) ;
				instrument_->ProcessCommand(channel_,table_->cmd2_[position_[1]],table_->param2_[position_[1]]) ;
				instrument_->ProcessCommand(channel_,table_->cmd3_[position_[2]],table_->param3_[position_[2]]) ;

				previous_[0]=position_[0] ;
				previous_[1]=position_[1] ;
				previous_[2]=position_[2] ;

			}

			// if groove's end reached, update position

			if (gs->UpdateGroove(groove_,true)) {

				if ((table_->cmd1_[position_[0]]!=I_CMD_HOP)||(!hopped_[0])) {
					position_[0]=(position_[0]+1)%16 ;
				}
				if ((table_->cmd2_[position_[1]]!=I_CMD_HOP)||(!hopped_[1])) {
					position_[1]=(position_[1]+1)%16 ;
				}
				if ((table_->cmd3_[position_[2]]!=I_CMD_HOP)||(!hopped_[2])) {
					position_[2]=(position_[2]+1)%16 ;
				}

				hopped_[0]=false ;
				hopped_[1]=false ;
				hopped_[2]=false ;

				if (automated_) {
					TableSaveState state ;
					memcpy(state.hopCount_,hopCount_,sizeof(uchar)*TABLE_STEPS*3) ;
					memcpy(state.position_,position_,sizeof(int)*3) ;
					instrument_->SetTableState(state) ;
				}
			}
		}
	}
}

