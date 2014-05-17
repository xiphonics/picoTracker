#ifndef _APP_TYPES_H_
#define _APP_TYPES_H_

//#define FOURCC(i) (((i&0xff000000)>>24) | ((i&0x00ff0000)>>8) | ((i&0x0000ff00)<<8) | ((i&0x000000ff)<<24))
#ifdef __ppc__
#define MAKE_FOURCC(ch0,ch1,ch2,ch3) (ch3 | ch2<<8 | ch1<<16 | ch0<<24)
#else
#define MAKE_FOURCC(ch0,ch1,ch2,ch3) (ch0 | ch1<<8 | ch2<<16 | ch3<<24)
#endif

#ifdef WIN32
#define strcasecmp _stricmp
#endif

typedef unsigned int FourCC ;
typedef unsigned short ushort ;
typedef unsigned int uint ;
typedef unsigned char uchar ;

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)<(b)?(a):(b))

#endif
