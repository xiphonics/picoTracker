#include <cstdio>
#include <cstring>

static char **getHelpLegend(FourCC command) {
  static char *result[2];
  result[1] = (char *)("bb at speed aa");
  switch (command) {
  case I_CMD_KILL:
    result[0] = (char *)("     KILl:--bb, stop playing    ");
    result[1] = (char *)("after bb ticks");
    break;
  case I_CMD_LPOF:
    result[0] = (char *)("     Loop OFset: Shift loop     ");
    result[1] = (char *)("start & end values aaaa");
    break;
  case I_CMD_ARPG:
    result[0] = (char *)("     ARPeggio: Cycle through    ");
    result[1] = (char *)("relative pitches");
    break;
  case I_CMD_VOLM:
    result[0] = (char *)("     VOLume:aabb, reach volume  ");
    break;
  case I_CMD_PTCH:
    result[0] = (char *)("     Pitch SLide:aabb, to pitch ");
    break;
  case I_CMD_HOP:
    result[0] = (char *)("     HOP:aabb, hop to bb        ");
    result[1] = (char *)("aa times");
    break;
  case I_CMD_LEGA:
    result[0] = (char *)("     LEGato:aabb, slide         ");
    result[1] = (char *)("to pitch bb at speed aa");
    break;
  case I_CMD_RTRG:
    result[0] = (char *)("     ReTriG:aabb retrigger loop ");
    result[1] = (char *)("over bb ticks at speed aa");
    break;
  case I_CMD_TMPO:
    result[0] = (char *)("     TemPO:--bb, set tempo to   ");
    result[1] = (char *)("hex value bb");
    break;
  case I_CMD_MDCC:
    result[0] = (char *)("     MidiCC:aabb, CC message aa ");
    result[1] = (char *)("value bb");
    break;
  case I_CMD_MDPG:
    result[0] = (char *)("     Midi Program Change:aabb,  ");
    result[1] = (char *)("send program change bb");
    break;
  case I_CMD_PLOF:
    result[0] = (char *)("     Play OFfset:aabb, jump abs ");
    result[1] = (char *)("to aa or move rel bb chunks");
    break;
  case I_CMD_FRES:
    result[0] = (char *)("     FiLTer&Res:aabb, cutoff aa ");
    result[1] = (char *)("resonance bb");
    break;
  case I_CMD_FLTR:
    result[0] = (char *)("     FiLTeR:aabb, resonance to  ");
    break;
  case I_CMD_TABL:
    result[0] = (char *)("     TaBLe:--bb, run table bb   ");
    result[1] = (char *)("");
    break;
  case I_CMD_CRSH:
    result[0] = (char *)("     drive&CruSH:aa-b, drive aa ");
    result[1] = (char *)("crush -b");
    break;
  case I_CMD_FCUT:
    result[0] = (char *)("     FilterCuToff:aabb, cutoff  ");
    break;
  case I_CMD_PAN_:
    result[0] = (char *)("     PAN:aabb, pan to value     ");
    break;
  case I_CMD_GROV:
    result[0] = (char *)("     GRooVe:--bb, set groove bb ");
    result[1] = (char *)("");
    break;
  case I_CMD_IRTG:
    result[0] = (char *)("     InstrumentReTrig:aabb, trig");
    result[1] = (char *)("& transpose to aa, speed bb ");
    break;
  case I_CMD_PFIN:
    result[0] = (char *)("     PitchFineTune:aabb, tune   ");
    break;
  case I_CMD_DLAY:
    result[0] = (char *)("     DeLaY:--bb, delay bb tics  ");
    result[1] = (char *)("");
    break;
  case I_CMD_STOP:
    result[0] = (char *)("     SToP playing song now      ");
    result[1] = (char *)("");
    break;
  default:
    result[0] = result[1] = (char *)("");
    break;
  }
  return result;
}
