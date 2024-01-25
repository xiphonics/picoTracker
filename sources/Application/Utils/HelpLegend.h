#include <cstdio>
#include <cstring>

static char** getHelpLegend(FourCC command) {
  static char* result[2];
  result[1] = (char*)("bb at speed aa");
  switch (command)
  {
    case 1:
      result[0] = (char*)("KILl: stop playing after");
      result[1] = (char*)("aa ticks");
      break;
    case 2:
      result[0] = (char*)("Loop OFset: Shift both loop");
      result[1] = (char*)("start & end values aaaa");
      break;
    case 3:
      result[0] = (char*)("ARPeggio: Cycle through");
      result[1] = (char*)("relative pitches");
      break;
    case 4:
      result[0] = (char*)("VOLume:aabb, reach volume");
      break;
    case 5:
      result[0] = (char*)("Pitch SLide:aabb, reach pitch");
      break;
    case 6:
      result[0] = (char*)("HOP:aabb, hop to bb");
      result[1] = (char*)("aa times");
      break;
    case 7:
      result[0] = (char*)("LEGato:aabb, slide");
      result[1] = (char*)("to pitch bb at speed aa");
      break;
    case 8:
      result[0] = (char*)("ReTriG:aabb retrigger loop");
      result[1] = (char*)("over bb ticks at speed aa");
      break;
    case 9:
      result[0] = (char*)("TemPO:--bb, set tempo to hex");
      result[1] = (char*)("value bb");
      break;
    case 10:
      result[0] = (char*)("MidiCC:aabb, CC message aa");
      result[1] = (char*)("value bb");
      break;
    case 11:
      result[0] = (char*)("Midi Program Change send");
      result[1] = (char*)("program change bb");
      break;
    case 12:
      result[0] = (char*)("Play OFfset:aabb, jump abs");
      result[1] = (char*)("to aa or move rel bb chunks");
      break;
    case 13:
      result[0] = (char*)("FiLTer&Res:aabb, cutoff aa");
      result[1] = (char*)("resonance bb");
      break;
    case 14:
      result[0] = (char*)("TaBLe:--bb, trigger table bb");
      result[1] = (char*)("");
      break;
    case 15:
      result[0] = (char*)("bitCruSH:aa-b, drive aa");
      result[1] = (char*)("crush -b");
      break;
    case 16:
      result[0] = (char*)("FilterCuToff:aabb, cutoff to");
      break;
    case 17:
      result[0] = (char*)("PAN:aabb, pan to value");
      break;
    case 18:
      result[0] = (char*)("GRooVe:--bb, set groove bb");
      result[1] = (char*)("");
      break;
    case 19:
      result[0] = (char*)("InstrumentReTrig:aabb, retrig");
      result[1] = (char*)("& transpose to aa at speed bb");
      break;
    case 20:
      result[0] = (char*)("PitchFineTune:aabb, tune to");
      break;
    case 21:
      result[0] = (char*)("DeLaY:--bb, delay bb tics");
      result[1] = (char*)("");
      break;
    case 22:
      result[0] = (char*)("SToP playing song immediately");
      result[1] = (char*)("");
      break;
    default:
      result[0] = result[1] = (char*)("");
      break;
    }
  return result;
}

