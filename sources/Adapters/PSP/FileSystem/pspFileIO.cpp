/*
 *  pspFileIO.cpp
 */

#include "avsb.h"

#ifdef PCBUILD
#include "pspFileIO.h"
#endif

Uint32 filelen;
char pg_workdir[MAX_PATH];

//
//  write bytes
//

void fwByte(Uint8 data, int fd)
{
	filelen=pgfWrite(fd,&data,1);
}

void fwByte(Uint8 data, FILE *outfile)
{
	fwrite(&data, sizeof(Uint8), 1, outfile);
}

//
//  write floats
//

void fwFloat4(float data, int fd)
{
	Uint8 *p_ch1;
	p_ch1 = (Uint8 *)&data;
	pgfWrite(fd,p_ch1,4);

	return;
}

void fwFloat4(float data, FILE *outfile)
{
	Uint8 ch1;
	Uint8 *p_ch1;

	p_ch1 = (Uint8 *)&data;

	for(int i=0; i<4; i++)
	{
		ch1 = *p_ch1++;
		fwrite(&ch1, sizeof(Uint8), 1, outfile);
	}

	return;
}

//
//  writes ints (32bits)
//

void fwInt(int data, int fd)
{
	Uint8 *p_ch1 = (Uint8 *)&data;
	filelen=pgfWrite(fd,p_ch1,4);
}

void fwInt(int data, FILE *outfile)
{
	Uint8 *p_ch1 = (Uint8 *)&data;
	fwrite(p_ch1, sizeof(Uint8), 4, outfile);
}

//
//  write shorts
//

void fwShort(int data, int fd)
{
	Uint8 *p_ch1 = (Uint8 *)&data;
	filelen=pgfWrite(fd,p_ch1,2);
}

void fwShort(int data, FILE *outfile)
{
	Uint8 *p_ch1 = (Uint8 *)&data;
	fwrite(p_ch1, sizeof(Uint8), 2, outfile);
}

//
//  read bytes
//

Uint8 frByte(int fd)
{
	Uint8 ch1;

	pgfRead(fd, &ch1, 1);
	return(ch1);
}

Uint8 frByte(FILE *infile)
{
	Uint8 ch1;
	fread(&ch1, sizeof(Uint8), 1, infile);
	return(ch1);
}

//
//  read floats
//

float frFloat4(int fd)
{
	float *p_fl1;

	Uint8 ch2[4];

	pgfRead(fd, &ch2[0], 4);
	p_fl1 = (float *)&ch2[0];
	
	return (*p_fl1);
}


float frFloat4(FILE *infile)
{

	float *p_fl1;

	Uint8 ch2[4];

	fread(&ch2[0], sizeof(Uint8), 1, infile);
	fread(&ch2[1], sizeof(Uint8), 1, infile);
	fread(&ch2[2], sizeof(Uint8), 1, infile);
	fread(&ch2[3], sizeof(Uint8), 1, infile);
	
	p_fl1 = (float *)&ch2[0];
	
	return (*p_fl1);
}

//
//  read ints
//

int frInt(int fd)
{
	int* p_i1;

	Uint8 ch2[4];

	pgfRead(fd, &ch2[0], 4);
	p_i1 = (int *)&ch2[0];
	
	return (*p_i1);
}

int frInt(FILE *infile)
{	
	int* p_i1;

	Uint8 ch2[4];

	fread(&ch2[0], sizeof(Uint8), 4, infile);
	p_i1 = (int *)&ch2[0];
	
	return (*p_i1);
}

//
//  read shorts
//

Uint16 frShort(int fd)
{

	Uint16* p_sh1;

	Uint8 ch2[2];

	pgfRead(fd, &ch2[0], 2);
	p_sh1 = (Uint16 *)&ch2[0];
	
	return (*p_sh1);

}

Uint16 frShort(FILE *infile)
{
	Uint16* p_sh1;
	Uint8 ch2[2];

	fread(&ch2, sizeof(Uint8), 2, infile);
	p_sh1 = (Uint16 *)&ch2[0];
	
	return (*p_sh1);
}



int pgfOpen(const char *filename, unsigned long flag)
{
   char fn[MAX_PATH*2];
   if (strchr(filename,':')!=NULL || *filename=='/' || *filename=='\\') {
#ifdef PSPBUILD
	   return sceIoOpen(filename,flag, 0777);
#else
	   return 1;
#endif
   } else {
      strcpy(fn,pg_workdir);
      strcat(fn,filename);
#ifdef PSPBUILD
      return sceIoOpen(fn,flag, 0777);
#else
	  return 1;
#endif
   }
}

#ifdef PSPBUILD
void pgfClose(int fd)
{
   sceIoClose(fd);
}
#endif
#ifdef PCBUILD
void pgfClose(FILE* fd)
{
   fclose(fd);
}
#endif

int pgfRead(int fd, void *data, int size)
{
#ifdef PSPBUILD
   return sceIoRead(fd,data,size);
#else
   return 1;
#endif
}

int pgfWrite(int fd, void *data, int size)
{
#ifdef PSPBUILD
   return sceIoWrite(fd,data,size);
#else
   return 1;
#endif
}

//
//  depreciated inefficient loads
//

#ifdef INCLUDE_SLOW_DATARW
void fwFloat(float data, int fd)
{
	int j;
	int i1, decbits;
	float flrem, remcheck, fltemp;
	Uint8 ch1;

	// write upper 32bits first
	i1 = (int)data;

	if(data < 0.0f && data > -1.0f)
	{
		i1 = i1 | 0x80000000;
	}

	for(j=0; j<4; j++)
	{
		ch1 = (i1 >> j*8) & 0x000000ff;
		filelen=pgfWrite(fd,&ch1,1);
	}

	if(data < 0.0f && data > -1.0f)
	{
		i1 = i1 & 0x7fffffff;
	}

	// get remainder
	flrem = data-(float)i1;
	if(flrem < 0.0f)
	{
		flrem = -flrem;
	}
		
	remcheck = 0.5f;
	decbits = 0;
	for(j=0; j<32; j++)
	{
		fltemp = flrem - remcheck;
		if(fltemp>=0.0f)
		{
			flrem = flrem - remcheck;
			decbits = decbits | (1<<(31-j));
		}
		remcheck = remcheck/2.0f;
	}
	for(j=0; j<4; j++)
	{
		ch1 = decbits>>((3-j)*8);
		filelen=pgfWrite(fd,&ch1,1);
	}
}


void fwFloat(float data, FILE *outfile)
{
	int j;
	int i1, decbits;
	float flrem, remcheck, fltemp;
	Uint8 ch1;

	// write upper 32bits first
	i1 = (int)data;
	if(data < 0.0f && data > -1.0f)
	{
		i1 = i1 | 0x80000000;
	}

	for(j=0; j<4; j++)
	{
		ch1 = (i1 >> j*8) & 0x000000ff;
		fwrite(&ch1, sizeof(Uint8), 1, outfile);
	}

	if(data < 0.0f && data > -1.0f)
	{
		i1 = i1 & 0x7fffffff;
	}

	// get remainder
	flrem = data-(float)i1;
	if(flrem < 0.0f)
	{
		flrem = -flrem;
	}
		
	remcheck = 0.5f;
	decbits = 0;
	for(j=0; j<32; j++)
	{
		fltemp = flrem - remcheck;
		if(fltemp>=0.0f)
		{
			flrem = flrem - remcheck;
			decbits = decbits | (1<<(31-j));
		}
		remcheck = remcheck/2.0f;
	}
	for(j=0; j<4; j++)
	{
		ch1 = decbits>>((3-j)*8);
		fwrite(&ch1, sizeof(Uint8), 1, outfile);
	}
}

float frFloat(int fd)
{
	Uint32 ui1, ui2, ui3;
	Sint32 si1;
	int k;
	Uint8 ch1;
	float fl1;

	ui1 = 0;  ui2 = 0;
	for(k=0; k<4; k++)
	{
	    pgfRead(fd, &ch1, 1);
		ui1 = ui1 | (ch1<<(k)*8);
	}

	for(k=0; k<4; k++)
	{
	    pgfRead(fd, &ch1, 1);
		ui3 = (ch1<<(3-k)*8);
		ui2 = ui2 | ui3;
	}
	fl1 = (float)ui2;
	fl1 = fl1/4294967296.0f;
	
	if(ui1 & 0x80000000)
	{
		if(ui1 == 0x80000000)
		{
			ui1 = 0x0;
		}
		si1 = (Sint32)ui1;
		si1 = -si1;
		fl1 = fl1 + float(si1);
		fl1 = -fl1;
	}
	else
	{
		fl1 = fl1 + (float)ui1;
	}
	
	return(fl1);
}

float frFloat(FILE *infile)
{

	Uint32 ui1, ui2, ui3;
	Sint32 si1;
	int k;
	Uint8 ch1;
	float fl1;

	ui1 = 0;  ui2 = 0;
	for(k=0; k<4; k++)
	{
	    fread(&ch1, sizeof(Uint8), 1, infile);
		ui1 = ui1 | (ch1<<(k)*8);
	}

	for(k=0; k<4; k++)
	{
	    fread(&ch1, sizeof(Uint8), 1, infile);
		ui3 = (ch1<<(3-k)*8);
		ui2 = ui2 | ui3;
	}
	fl1 = (float)ui2;
	fl1 = fl1/4294967296.0f;
	
	if(ui1 & 0x80000000)
	{
		if(ui1 == 0x80000000)
		{
			ui1 = 0x0;
		}
		si1 = (Sint32)ui1;
		si1 = -si1;
		fl1 = fl1 + float(si1);
		fl1 = -fl1;
	}
	else
	{
		fl1 = fl1 + (float)ui1;
	}
	
	return(fl1);
}
#endif
