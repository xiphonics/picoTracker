
#ifndef _COMMAND_LIST_H_
#define _COMMAND_LIST_H_

#include "Foundation/Types/Types.h"

#define I_CMD_NONE MAKE_FOURCC('-','-','-','-')
#define I_CMD_KILL MAKE_FOURCC('K','I','L','L')
#define I_CMD_LPOF MAKE_FOURCC('L','P','O','F')
#define I_CMD_ARPG MAKE_FOURCC('A','R','P','G')
#define I_CMD_VOLM MAKE_FOURCC('V','O','L','M')
#define I_CMD_PTCH MAKE_FOURCC('P','T','C','H')
#define I_CMD_HOP  MAKE_FOURCC('H','O','P',' ')
#define I_CMD_LEGA MAKE_FOURCC('L','E','G','A')
#define I_CMD_RTRG MAKE_FOURCC('R','T','R','G')
#define I_CMD_TMPO MAKE_FOURCC('T','M','P','O')
#define I_CMD_MDCC MAKE_FOURCC('M','D','C','C')
#define I_CMD_MDPG MAKE_FOURCC('M','D','P','G')
#define I_CMD_PLOF MAKE_FOURCC('P','L','O','F')
#define I_CMD_FLTR MAKE_FOURCC('F','L','T','R')
#define I_CMD_TABL MAKE_FOURCC('T','A','B','L')
#define I_CMD_CRSH MAKE_FOURCC('C','R','S','H')
#define I_CMD_FCUT MAKE_FOURCC('F','C','U','T')
#define I_CMD_FRES MAKE_FOURCC('F','R','E','S')
#define I_CMD_PAN_ MAKE_FOURCC('P','A','N',' ')
#define I_CMD_GROV MAKE_FOURCC('G','R','O','V')
#define I_CMD_FBTU MAKE_FOURCC('F','B','T','U')
#define I_CMD_FBAM MAKE_FOURCC('F','B','A','M')
#define I_CMD_IRTG MAKE_FOURCC('I','R','T','G')
#define I_CMD_PFIN MAKE_FOURCC('P','F','I','N')
#define I_CMD_DLAY MAKE_FOURCC('D','L','A','Y')
#define I_CMD_FBMX MAKE_FOURCC('F','B','M','X')
#define I_CMD_FBTN MAKE_FOURCC('F','B','T','N')
#define I_CMD_STOP MAKE_FOURCC('S','T','O','P')

class CommandList {
public:
	static FourCC GetNext(FourCC current) ;
	static FourCC GetPrev(FourCC current) ;
	static FourCC GetNextAlpha(FourCC current) ;
	static FourCC GetPrevAlpha(FourCC current) ;};
#endif

