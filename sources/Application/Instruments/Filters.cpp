/****************************
some wanabe filter code by Beausoleil S Guillemette
hope it will work :D
[2006-09-25 .. 2006-09-30]
****************************/

#include "Filters.h"
#include <math.h>
#include "System/Console/Trace.h"

static filter_t filter[8];

bool filters_inited=false;

fixed fp_inv_255 ;

void init_filters(void)
{

	for(int i=0;i<8;i++)
	{	//set sensible default values
		//lowpass filter where everything passes with no resonance
		set_filter(i,FLT_LOWPASS,i2fp(1),i2fp(0),i2fp(0),false);
	}
	fp_inv_255=fl2fp(1.0f/255) ;
	filters_inited=true;
}

void set_filter(int channel, filterType_t type, fixed param1,fixed param2,int mix,bool bassyMapping)
{
	filter_t* flt=&filter[channel];

	if(flt->type!=type)		//reset only on filter type change   (maybe no reset would make interresting bugs)
	{
		flt->height[0]=0;		//idle state
		flt->height[1]=0;		//idle state
		flt->speed[0]=0;		//paused	(to avoid having it goind crazy)
		flt->speed[1]=0;		//paused	(to avoid having it goind crazy)
		flt->hipdelay[0]=0 ;
		flt->hipdelay[1]=0 ;
		flt->type=type;
	}
  
  
	flt->dirt=fp_mul(i2fp(100),i2fp(1)-param1)+fp_mul(i2fp(5000),param1) ;
	flt->mix =fp_mul(i2fp(mix),fp_inv_255) ;

  if (param1 != flt->parm1)
  {
  	flt->parm1=param1;
    //adjust parm to get the most of the parameters, as the fx are more useful with near-limit parameters.
    if (bassyMapping)
    {
      static const fixed fpFreqDivider = fl2fp(1/22050.0f);
      static const fixed fpZeroSix = fl2fp(0.6f);
      static const fixed fpThreeOne = fl2fp(3.1f);

      fixed power = fp_add(fpZeroSix,fp_mul(param1,fpThreeOne));
      fixed frequency = fl2fp(pow(10.0f,fp2fl(power))) ;
      frequency=fp_mul(frequency,fpFreqDivider);
      flt->freq=frequency;
    }
    else
    {
      flt->freq=fp_mul(param1,param1);				//0 - .5 - 1   =>   0 - .25 - 1
    }    
  }
  
  if (param2 != flt->parm2)
  {
    flt->parm2=param2;
    flt->reso=i2fp(1)-param2 ;
    flt->reso=fp_sub(fl2fp(1.f),fp_mul(flt->reso,fp_mul(flt->reso,flt->reso)));	//0 - .5 - 1   =>   0 - .93 - 1
  }
}


filter_t *get_filter(int channel) {
	return &filter[channel];
} ;
/*
void filterize(int channel, fixed* buffer, long int size)
{
	if(!filters_inited)init_filters();

	fixed* buf=buffer;
	filter_t* flt = &filter[channel];

	for(int i=0;i<size;i++)					//there's only lowpass for now. so they'll all sound the same.
	{
		fixed *l=buf++;
		fixed *r=buf++;

		fixed goalr = *r;
		fixed goall = *l;

		fixed difr = fp_sub(goalr,flt->height[0]);
		fixed difl = fp_sub(goall,flt->height[1]);

		flt->speed[0] = fp_mul(flt->speed[0],flt->parm2);		//mul by res, it's some kind of inertia. caution to feedback
		flt->speed[1] = fp_mul(flt->speed[1],flt->parm2);

		flt->speed[0] = fp_add(flt->speed[0],fp_mul(difr,flt->parm1));	//mul by cutoff, less cutoff = no sound, so it's better not be 0.
		flt->speed[1] = fp_add(flt->speed[1],fp_mul(difl,flt->parm1));

		flt->height[0] = fp_add(flt->height[0],flt->speed[0]);
		flt->height[1] = fp_add(flt->height[1],flt->speed[1]);

		*r = flt->height[0];
		*l = flt->height[1];
	}
}
*/
/*
void filterize(int channel, void* buffer, long int size)
{
	if(!filters_inited)init_filters();

	int* buf=(int*)buffer;
	filter_t* flt = &filter[channel];

	for(int i=0;i<size;i++)					//there's only lowpass for now. so they'll all sound the same.
	{
		short* l;l = (short*)&buf[i];
		short* r;r = (short*)&buf[i];
		r++;

		float goalr = *r;
		float goall = *l;

		float difr = goalr - flt->height[0];
		float difl = goall - flt->height[1];

		flt->speed[0] *= flt->parm2;		//mul by res, it's some kind of inertia. caution to feedback
		flt->speed[1] *= flt->parm2;

		flt->speed[0] += difr*flt->parm1;	//mul by cutoff, less cutoff = no sound, so it's better not be 0.
		flt->speed[1] += difl*flt->parm1;

		flt->height[0] += flt->speed[0];
		flt->height[1] += flt->speed[1];

		*r = (short)flt->height[0];
		*l = (short)flt->height[1];
	}
}
*/
