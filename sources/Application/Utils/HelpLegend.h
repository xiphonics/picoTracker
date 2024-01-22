#include <cstdio>
#include <cstring>

static inline const char (*getHelpLegend(const char *fx))[30] {
  static char result[3][30];
  strcpy(result[0], fx);
  strcpy(result[2], "bb at speed aa");
  if (strcmp(fx, "KIL") == 0) {
    strcat(result[0], ", KILl: stop playing after");
    strcpy(result[1], "aa ticks");
    result[2][0] = '\0';
  } else if (strcmp(fx, "LOF") == 0) {
    strcat(result[0], ", Loop OFset: Shift both");
    strcpy(result[1], "the loop start & loop ");
    strcpy(result[2], "end values aaaa digits");
  } else if (strcmp(fx, "ARP") == 0) {
    strcat(result[0], ", ARPeggio: Cycle");
    strcpy(result[1], "through relative pitches");
    strcpy(result[2], "from original pitch");
  } else if (strcmp(fx, "VOL") == 0) {
    strcat(result[0], ", VOLume:aabb");
    strcpy(result[1], "approach volume");
  } else if (strcmp(fx, "PSL") == 0) {
    strcat(result[0], ", Pitch SLide:aabb");
    strcpy(result[1], "approach pitch");
  } else if (strcmp(fx, "HOP") == 0) {
    strcat(result[0], ":aabb");
    strcpy(result[1], "hop to bb");
    strcpy(result[2], "aa times");
  } else if (strcmp(fx, "LEG") == 0) {
    strcat(result[0], ", LEGato: slide from");
    strcpy(result[1], "previous note to pitch");
  } else if (strcmp(fx, "RTG") == 0) {
    strcat(result[0], ", ReTriG: retrigger loop");
    strcpy(result[1], "from current position over");
    strcpy(result[2], "bb ticks at speed aa");
  } else if (strcmp(fx, "TPO") == 0) {
    strcat(result[0], ", TemPO:--bb");
    strcpy(result[1], "sets the tempo to hex");
    strcpy(result[2], "value bb");
  } else if (strcmp(fx, "MCC") == 0) {
    strcat(result[0], ", MidiCC:aabb");
    strcpy(result[1], "CC message aa");
    strcpy(result[2], "value bb");
  } else if (strcmp(fx, "MPC") == 0) {
    strcat(result[0], ", Midi Program Change");
    strcpy(result[1], "send program change bb");
    strcpy(result[2], "to current channel");
  } else if (strcmp(fx, "POF") == 0) {
    strcat(result[0], ", Play OFfset:aabb");
    strcpy(result[1], "jump abs to aa or");
    strcpy(result[2], "move rel bb chunks");
  } else if (strcmp(fx, "FLT") == 0) {
    strcat(result[0], ", FiLTer&Resonance:aabb");
    strcpy(result[1], "cutoff aa");
    strcpy(result[2], "resonance bb");
  } else if (strcmp(fx, "TBL") == 0) {
    strcat(result[0], ", TaBLe:--bb");
    strcpy(result[1], "trigger table bb");
    result[2][0] = '\0';
  } else if (strcmp(fx, "CSH") == 0) {
    strcat(result[0], ", bitCruSH:aa-b");
    strcpy(result[1], "drive aa");
    strcpy(result[2], "crush -b");
  } else if (strcmp(fx, "FCT") == 0) {
    strcat(result[0], ", FilterCuToff:aabb");
    strcpy(result[1], "set cutoff to");
  } else if (strcmp(fx, "FRS") == 0) {
    strcat(result[0], ", FilterReSonance:aabb");
    strcpy(result[1], "set resonance to");
  } else if (strcmp(fx, "PAN") == 0) {
    strcat(result[0], ", PAN:aabb");
    strcpy(result[1], "pan to value");
  } else if (strcmp(fx, "GRV") == 0) {
    strcat(result[0], ", GRooVe:--bb");
    strcpy(result[1], "trigger groove bb");
    result[2][0] = '\0';
  } else if (strcmp(fx, "IRT") == 0) {
    strcat(result[0], ", InstrumentReTrig:aabb");
    strcpy(result[1], "retrig and transpose to");
  } else if (strcmp(fx, "PFT") == 0) {
    strcat(result[0], ", PitchFineTune:aabb");
    strcpy(result[1], "fine tune to ");
  } else if (strcmp(fx, "DLY") == 0) {
    strcat(result[0], ", DeLaY:--bb");
    strcpy(result[1], "delay bb tics");
    result[2][0] = '\0';
  } else if (strcmp(fx, "STP") == 0) {
    strcat(result[0], ", SToP playing song");
    strcpy(result[1], "immediately");
    result[2][0] = '\0';
  } else {
    result[0][0] = '\0';
    result[1][0] = '\0';
    result[2][0] = '\0';
  }
  return result;
}
