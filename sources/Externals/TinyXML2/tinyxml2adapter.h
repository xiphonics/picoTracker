#ifndef _TINY2NOSSTUP_H_
#define _TINY2NOSSTUP_H_
#include "System/Console/n_assert.h"
#include "System/FileSystem/PicoFileSystem.h"
#include <stdio.h>

#ifdef FILE
#undef FILE
#endif
#define FILE PI_File
#define fopen(a, b) PicoFileSystem::GetInstance()->Open(a, b)
#define fclose(a)  a->Close() ; delete (a)
#ifdef __APPLE__
#define fseeko(a, b, c) a->Seek(b, c)
#define ftello(a) a->Tell()
#else
#define fseek(a, b, c) a->Seek(b, c)
#define ftell(a) a->Tell()
#endif
#define fputs(a, b) b->Write(a, 1, strlen(a))
#define fputc(a,b) b->Write(&a,1,1)
#define fread(a,b,c,d)  d->Read(a,b,c)
#define fwrite(a,b,c,d) d->Write(a,b,c)
#define fgetc(a) a->GetC()
#define ferror(a) a->Error()
extern void fprintf(FILE *f, const char *fmt,...) ;
#define vfprintf(a, b, c) fprintf(a, b, c)
#endif
