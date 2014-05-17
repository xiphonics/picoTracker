
#include "CommandList.h"

static FourCC _all[]= {
	I_CMD_NONE,
	I_CMD_ARPG,
	I_CMD_CRSH,
	I_CMD_DLAY,
	I_CMD_FBMX,
	I_CMD_FBTN,
	I_CMD_FCUT,
	I_CMD_FLTR,
	I_CMD_FRES,
	I_CMD_GROV,
	I_CMD_HOP,
	I_CMD_IRTG,
	I_CMD_KILL,
	I_CMD_LEGA,
	I_CMD_LPOF,
	I_CMD_MDCC,
	I_CMD_MDPG,
	I_CMD_PAN_,
	I_CMD_PFIN,
	I_CMD_PLOF,
	I_CMD_PTCH,
	I_CMD_RTRG,
  I_CMD_STOP,
	I_CMD_TABL,
	I_CMD_TMPO,
	I_CMD_VOLM
} ;

FourCC CommandList::GetNext(FourCC current) {
	for (uint i=0;i<sizeof(_all)/sizeof(FourCC)-1;i++) {
		if (_all[i]==current) {
			return _all[i+1] ;
		} ;
	} ;
	return current ;
} ;

FourCC CommandList::GetPrev(FourCC current) {
    uint count=sizeof(_all)/sizeof(FourCC) ;
	for (uint i=2;i<count;i++) {
		if (_all[i]==current) {
			return _all[i-1] ;
		} ;
	} ;
	return current ;
} ;

FourCC CommandList::GetNextAlpha(FourCC current) {
	char letter=((char *)&current)[0];
	bool found=false ;
	for (uint i=0;i<sizeof(_all)/sizeof(FourCC);i++) {
		char tLetter=((char *)&_all[i])[0];
		if (!found) {
			if (tLetter==letter) {
				found=true ;
			}
		} else {
			if (tLetter!=letter) {
				return _all[i] ;
			}
		} ;
	} ;
	return current ;
} ;

FourCC CommandList::GetPrevAlpha(FourCC current) {

	char letter=((char *)&current)[0];
	bool found=false ;
	FourCC tReturn=0xFFFFFFFF ;
	uint count=sizeof(_all)/sizeof(FourCC) ;

	for (uint i=count-1;i>0;i--) {
		char tLetter=((char *)&_all[i])[0];
		if (!found) {
			if (tLetter==letter) {
				found=true ;
			}
		} else {
			if (tLetter!=letter) {
				if (tReturn==0xFFFFFFFF) {
					tReturn=_all[i] ;
				} else {
					if (tLetter!=((char *)&tReturn)[0]) {
						return tReturn ;
					} else {
						tReturn=_all[i] ;
					}
				}
			}
		} ;
	} ;
	if (tReturn!=0xFFFFFFFF) {
		return tReturn ;
	} 
	return current ;
} ;
