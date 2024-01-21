#ifndef _HELP_LEGEND_H_
#define _HELP_LEGEND_H_

#include <string>
#include <stdlib.h>
#include <string.h>

class HelpLegend {
  public:
	static inline std::string* getHelpLegend(char* fx);
};

static inline std::string* getHelpLegend(char* fx) {
	std::string* result = new std::string[3];
	result[0].assign(fx);
	result[2].assign("bb at speed aa");
	if (strcmp(fx, "KILL") == 0) {
		result[0].append(", stop playing after");
		result[1].assign("aa ticks");
		result[2].assign("");
	} else if (strcmp(fx, "LPOF") == 0) {
		result[0].append(", LooP OFset: Shift both");
		result[1].assign("the loop start & loop ");
		result[2].assign("end values aaaa digits");
	} else if (strcmp(fx, "ARPG") == 0) {
		result[0].append(", ARPeGgio: Cycle");
		result[1].assign("through relative pitches");
		result[2].assign("from original pitch");
	} else if (strcmp(fx, "VOLM") == 0) {
		result[0].append(", VOLuMe:aabb");
		result[1].assign("approach volume");
	} else if (strcmp(fx, "PTCH") == 0) {
		result[0].append(", PiTCH:aabb");
		result[1].assign("approach pitch");
	} else if (strcmp(fx, "HOP ") == 0) {
		result[0].append(":aabb");
		result[1].assign("hop to bb");
		result[2].assign("aa times");
	} else if (strcmp(fx, "LEGA") == 0) {
		result[0].append(", LEGAto: slide from");
		result[1].assign("previous note to pitch");
	} else if (strcmp(fx, "RTRG") == 0) {
		result[0].append(", ReTRiG: retrigger loop");
		result[1].assign("from current position over");
		result[2].assign("bb ticks at speed aa");
	} else if (strcmp(fx, "TMPO") == 0) {
		result[0].append(", TeMPO:--bb");
		result[1].assign("sets the tempo to hex");
		result[2].assign("value bb");
	} else if (strcmp(fx, "MDCC") == 0) {
		result[0].append(", MiDiCC:aabb");
		result[1].assign("CC message aa");
		result[2].assign("value bb");
	} else if (strcmp(fx, "MDPG") == 0) {
		result[0].append(", MiDi ProGram Change");
		result[1].assign("send program change bb");
		result[2].assign("to current channel");
	} else if (strcmp(fx, "PLOF") == 0) {
		result[0].append(", PLay OFfset:aabb");
		result[1].assign("jump abs to aa or");
		result[2].assign("move rel bb chunks");
	} else if (strcmp(fx, "FLTR") == 0) {
		result[0].append(", FiLTeR:aabb");
		result[1].assign("cutoff aa");
		result[2].assign("resonance bb");
	} else if (strcmp(fx, "TABL") == 0) {
		result[0].append(", TABLe:--bb");
		result[1].assign("trigger table bb");
		result[2].assign("");
	} else if (strcmp(fx, "CRSH") == 0) {
		result[0].append(", CRuSH:aa-b");
		result[1].assign("drive aa");
		result[2].assign("crush -b");
	} else if (strcmp(fx, "FCUT") == 0) {
		result[0].append(", FilterCUToff:aabb");
		result[1].assign("set cutoff to");
	} else if (strcmp(fx, "FRES") == 0) {
		result[0].append(", FilterRESonance:aabb");
		result[1].assign("set resonance to");
	} else if (strcmp(fx, "PAN ") == 0) {
		result[0].append(", PAN:aabb");
		result[1].assign("pan to value");
	} else if (strcmp(fx, "GROV") == 0) {
		result[0].append(", GROoVe:--bb");
		result[1].assign("trigger groove bb");
		result[2].assign("");
	} else if (strcmp(fx, "IRTG") == 0) {
		result[0].append(", InstrumentReTriG:aabb");
		result[1].assign("retrig and transpose to");
	} else if (strcmp(fx, "PFIN") == 0) {
		result[0].append(", PitchFINetune:aabb");
		result[1].assign("fine tune to ");
	} else if (strcmp(fx, "DLAY") == 0) {
		result[0].append(", DeLAY:--bb");
		result[1].assign("delay bb tics");
		result[2].assign("");
	} else if (strcmp(fx, "FBMX") == 0) {
		result[0].append(", FeedBack MiX:aabb");
		result[1].assign("feedback mix to");
	} else if (strcmp(fx, "FBTN") == 0) {
		result[0].append(", FeedBack TuNe:aabb");
		result[1].assign("feedback tune to");
	} else if (strcmp(fx, "STOP") == 0) {
		result[0].append(" playing song");
		result[1].assign("immediately");
		result[2].assign("");
	} else {
		result[0].assign("");
		result[1].assign("");
		result[2].assign("");
	}
	return result;
}

#endif //_HELP_LEGEND_H_
