#include "ViewData.h"
#include "BaseClasses/View.h"

ViewData::ViewData(Project *project) {

	project_=project ;
	song_=project->song_ ;
	currentChain_=0 ;
	currentPhrase_=0 ;
	songX_=0 ;
	songY_=0 ;
	songOffset_=0 ;
	chainCol_=0 ;
	chainRow_=0 ;
	currentTable_=0 ;
	currentInstrument_=0 ;
	currentGroove_=0 ;
	mixerCol_=0 ;
	mixerRow_=0 ;
} ;

ViewData::~ViewData() {
	delete project_ ;
} ;

unsigned char ViewData::UpdateSongChain(int offset) {
	unsigned char *c=song_->data_+songX_+8*(songOffset_+songY_) ;
	updateData(c,offset,CHAIN_COUNT-1,false) ;
	return *c ;

}

void ViewData::SetSongChain(unsigned char value) {
	unsigned char *c=song_->data_+songX_+8*(songOffset_+songY_) ;
	*c=value ;
}

void ViewData::UpdateSongOffset(int offset) {
	songOffset_+=offset ;
	checkSongBoundaries() ;
}

void ViewData::UpdateSongCursor(int dx,int dy) {
	songX_+=dx;
	songY_+=dy ;
	checkSongBoundaries() ;
}

void ViewData::checkSongBoundaries() {
	if (songX_>7) songX_=7 ;
	if (songX_<0) songX_=0 ;
	if (songY_<0) {
		songOffset_+=songY_ ;
		songY_=0 ;
	} ;
	if (songY_>View::songRowCount_-1) {
		songOffset_+=songY_-View::songRowCount_+1 ;
		songY_=View::songRowCount_-1 ;
	} ;
	if (songOffset_>232) {
		songOffset_=232 ;
	}
	if (songOffset_<0) {
		songOffset_=0 ;
	}
}

unsigned char *ViewData::GetCurrentSongPointer() {
	return song_->data_+songX_+8*(songOffset_+songY_) ;
} ;


unsigned char ViewData::UpdateChainCursorValue(int offset,int dx,int dy) {

	unsigned char *c=0 ;
	unsigned char limit=0 ;
	bool wrap=false ;

	switch (chainCol_+dx) {
		case 0:
			c=song_->chain_->data_+(16*currentChain_+chainRow_+dy) ;
			limit=0xFE ;
			wrap=false ;
			break ;
		case 1:
			c=song_->chain_->transpose_+(16*currentChain_+chainRow_+dy) ;
			limit=0xFF ;
			wrap=true;
			break ;
	} 
	updateData(c,offset,limit,wrap) ;
	return *c ;
}

void ViewData::UpdateChainCursor(int dx,int dy) {
	chainCol_+=dx ;
	chainRow_+=dy ;
	if (chainCol_>1) chainCol_=1 ;
	if (chainCol_<0) chainCol_=0 ;
	if (chainRow_>15) chainRow_=15 ;
	if (chainRow_<0) chainRow_=0 ;
}

void ViewData::SetChainPhrase(unsigned char value) {
	unsigned char *c=song_->chain_->data_+(16*currentChain_+chainRow_) ;
	*c=value ;
}

unsigned char *ViewData::GetCurrentChainPointer() {
	return song_->chain_->data_+(16*currentChain_+chainRow_) ;
} ;

