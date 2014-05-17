#ifndef _TINY2NOSSTUP_H_
#define _TINY2NOSSTUP_H_
#include <stdio.h>
#include "System/FileSystem/FileSystem.h"
#include "System/Console/n_assert.h"

#ifdef FILE
#undef FILE
#endif
#define FILE I_File
#define fopen(a,b)  FileSystem::GetInstance()->Open(a,b)
#define fclose(a)  a->Close() ; delete (a)
#define fseek(a,b,c) a->Seek(b,c)
#define ftell(a) a->Tell()
#define fputs(a,b) b->Write(a,1,strlen(a))
#define fread(a,b,c,d)  d->Read(a,b,c)
extern void fprintf(FILE *f,char *fmt,...) ;

#endif
