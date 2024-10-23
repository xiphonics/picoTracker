#include <cstdio>
#include <cstring>

static char **getHelpLegend(FourCC command) {
  static char *result[2];
  result[1] = (char *)("bb at speed aa");
  switch (command) {
  case FourCC::InstrumentCommandKill:
    result[0] = (char *)("     KILl:--bb, stop playing    ");
    result[1] = (char *)("after bb ticks");
    break;
  case FourCC::InstrumentCommandLoopOfset:
    result[0] = (char *)("     Loop OFset: Shift loop     ");
    result[1] = (char *)("start & end values aaaa");
    break;
  case FourCC::InstrumentCommandArpeggiator:
    result[0] = (char *)("     ARPeggio: Cycle through    ");
    result[1] = (char *)("relative pitches");
    break;
  case FourCC::InstrumentCommandVolume:
    result[0] = (char *)("     VOLume:aabb, reach volume  ");
    break;
  case FourCC::InstrumentCommandVelocity:
    result[0] = (char *)("     VELocity:--bb, midi velocity ");
    break;
  case FourCC::InstrumentCommandPitchSlide:
    result[0] = (char *)("     Pitch SLide:aabb, to pitch ");
    break;
  case FourCC::InstrumentCommandHop:
    result[0] = (char *)("     HOP:aabb, hop to bb        ");
    result[1] = (char *)("aa times");
    break;
  case FourCC::InstrumentCommandLegato:
    result[0] = (char *)("     LEGato:aabb, slide         ");
    result[1] = (char *)("to pitch bb at speed aa");
    break;
  case FourCC::InstrumentCommandRetrigger:
    result[0] = (char *)("     ReTriG:aabb retrigger loop ");
    result[1] = (char *)("over bb ticks at speed aa");
    break;
  case FourCC::InstrumentCommandTempo:
    result[0] = (char *)("     TemPO:--bb, set tempo to   ");
    result[1] = (char *)("hex value bb");
    break;
  case FourCC::InstrumentCommandMidiCC:
    result[0] = (char *)("     MidiCC:aabb, CC message aa ");
    result[1] = (char *)("value bb");
    break;
  case FourCC::InstrumentCommandMidiPC:
    result[0] = (char *)("     Midi Program Change:aabb,  ");
    result[1] = (char *)("send program change bb");
    break;
  case FourCC::InstrumentCommandPlayOfset:
    result[0] = (char *)("     Play OFfset:aabb, jump abs ");
    result[1] = (char *)("to aa or move rel bb chunks");
    break;
  case FourCC::InstrumentCommandFilterResonance:
    result[0] = (char *)("     FiLTer&Res:aabb, cutoff aa ");
    result[1] = (char *)("resonance bb");
    break;
  case FourCC::InstrumentCommandLowPassFilter:
    result[0] = (char *)("     FiLTeR:aabb, resonance to  ");
    break;
  case FourCC::InstrumentCommandTable:
    result[0] = (char *)("     TaBLe:--bb, run table bb   ");
    result[1] = (char *)("");
    break;
  case FourCC::InstrumentCommandCrush:
    result[0] = (char *)("     drive&CruSH:aa-b, drive aa ");
    result[1] = (char *)("crush -b");
    break;
  case FourCC::InstrumentCommandFilterCut:
    result[0] = (char *)("     FilterCuToff:aabb, cutoff  ");
    break;
  case FourCC::InstrumentCommandPan:
    result[0] = (char *)("     PAN:aabb, pan to value     ");
    break;
  case FourCC::InstrumentCommandGroove:
    result[0] = (char *)("     GRooVe:--bb, set groove bb ");
    result[1] = (char *)("");
    break;
  case FourCC::InstrumentCommandInstrumentRetrigger:
    result[0] = (char *)("     InstrumentReTrig:aabb, trig");
    result[1] = (char *)("& transpose to aa, speed bb ");
    break;
  case FourCC::InstrumentCommandPitchFineTune:
    result[0] = (char *)("     PitchFineTune:aabb, tune   ");
    break;
  case FourCC::InstrumentCommandDelay:
    result[0] = (char *)("     DeLaY:--bb, delay bb tics  ");
    result[1] = (char *)("");
    break;
  case FourCC::InstrumentCommandStop:
    result[0] = (char *)("     SToP playing song now      ");
    result[1] = (char *)("");
    break;
  default:
    result[0] = result[1] = (char *)("");
    break;
  }
  return result;
}
