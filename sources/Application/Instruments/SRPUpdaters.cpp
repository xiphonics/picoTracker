
#include "SRPUpdaters.h"
#include <math.h>
#include "System/Console/Trace.h"
//
// Volume Ramp
//

void VolumeRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	speed_=(speed==0)?0:fl2fp(speed) ;
	current_=fl2fp(start) ;
} ;

void VolumeRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				current_=fp_sub(current_,speed_) ;
				if (current_<target_) {
					current_=target_ ;
				}
			} ;
		}
	} ;
};

void VolumeRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.volumeOffset_=fp_add(rup.volumeOffset_,current_) ;
} ;

//
// FilterCut off ramp
//

void FCRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	speed_=(speed==0)?0:fl2fp(speed) ;
	current_=fl2fp(start) ;
} ;

void FCRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				current_=fp_sub(current_,speed_) ;
				if (current_<target_) {
					current_=target_ ;
				}
			} ;
		}
	} ;
};

void FCRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.cutOffset_=fp_add(rup.cutOffset_,current_) ;
} ;

//
// Filter Resonance ramp
//

void FRRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	speed_=(speed==0)?0:fl2fp(speed) ;
	current_=fl2fp(start) ;
} ;

void FRRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				current_=fp_sub(current_,speed_) ;
				if (current_<target_) {
					current_=target_ ;
				}
			} ;
		}
	} ;
};

void FRRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.resOffset_=fp_add(rup.resOffset_,current_) ;
} ;

// Feedmack mix ramp

void FBMixRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	speed_=(speed==0)?0:fl2fp(speed) ;
	current_=fl2fp(start) ;
} ;

void FBMixRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				current_=fp_sub(current_,speed_) ;
				if (current_<target_) {
					current_=target_ ;
				}
			} ;
		}
	} ;
};

void FBMixRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.fbMixOffset_=fp_add(rup.fbMixOffset_,current_) ;
} ;

// Feedmack tune ramp

void FBTunRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	speed_=(speed==0)?0:fl2fp(speed) ;
	current_=fl2fp(start) ;
} ;

void FBTunRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				current_=fp_sub(current_,speed_) ;
				if (current_<target_) {
					current_=target_ ;
				}
			} ;
		}
	} ;
};

void FBTunRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.fbTunOffset_=fp_add(rup.fbTunOffset_,current_) ;
} ;

//
// Speed/Frequency Ramp
//

void LogSpeedRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	current_=fl2fp(start) ;
	if (target_>current_) {
		speed_=fl2fp(speed) ;
	} else {
		if (speed==0) {
			speed_=0 ;
		} else {
			speed_=fl2fp(1.0F/speed) ;
		}
	}
} ;

float LogSpeedRamp::GetCurrent() {
	return fp2fl(current_) ;
} ;

void LogSpeedRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
//			if (current_<target_) {
			if (speed_>FP_ONE) {
				current_=fp_mul(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
					speed_=0 ;
				}
			} else {
				if (current_>target_) {
					current_=fp_mul(current_,speed_) ;
					if (current_<target_) {
						current_=target_ ;
					}
				}
			} ;
		}
	} ;
};

void LogSpeedRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.speedOffset_=fp_mul(rup.speedOffset_,current_) ;
//	Trace::Debug("Log: current=%f,offset now=%f",fp2fl(current_),fp2fl(rup.speedOffset_)) ;
} ;

//
// Linear Speed/Frequency Ramp
//

void LinSpeedRamp::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	current_=fl2fp(start) ;

	speed_=(speed==0)?0:fl2fp(speed);  
} ;

void LinSpeedRamp::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				if (current_>target_) {
					current_=fp_sub(current_,speed_) ;
					if (current_<target_) {
						current_=target_ ;
					}
				}
			} ;
		}
	} ;
};

void LinSpeedRamp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.speedOffset_=fp_mul(rup.speedOffset_,current_) ;
} ;


//
// Panner
//

void Panner::SetData(float target,float speed,float start) {
	target_=fl2fp(target) ;
	current_=fl2fp(start) ;

	speed_=(speed==0)?0:fl2fp(speed);  
} ;

void Panner::Trigger(bool tableTick) {
	if (!enabled_) return ;
	if (!tableTick) {
		if (speed_==0) {
			current_=target_ ;
		} else {
			if (current_<target_) {
				current_=fp_add(current_,speed_) ;
				if (current_>target_) {
					current_=target_ ;
				}
			} else {
				if (current_>target_) {
					current_=fp_sub(current_,speed_) ;
					if (current_<target_) {
						current_=target_ ;
					}
				}
			} ;
		}
	} ;
};

void Panner::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.panOffset_=fp_add(rup.panOffset_,current_) ;
} ;

//
// Arpegiator
//

void Arp::SetData(unsigned int value) {
	int arp=value ;
	int position=0 ;
	arp_[0]=0;
	for (int i=0;i<4;i++) {
		arp_[4-i]=(arp&0xF) ;
		if ((arp_[4-i]!=0)&&(position==0)) {
			position=5-i ;
		}
		arp=(arp&0xFFF0)>>4 ;
	}
	arpLength_=position ;
	arpPosition_=0 ;
	current_=fl2fp(1.0) ;
} ;

void Arp::Trigger(bool tableTick) {

	if((!tableTick)||(!enabled_)) return ;
	 if (arpLength_>0) {
		arpPosition_++ ;
		if (arpPosition_>arpLength_) {
			arpPosition_=0 ;
		} ;
		current_=fl2fp(float(pow(2.0,arp_[arpPosition_]/12.0))) ;
	}
} ;

void Arp::UpdateSRP(struct RUParams &rup) {
	if (!enabled_) return ;
	rup.speedOffset_=fp_mul(rup.speedOffset_,current_) ;
} ;

