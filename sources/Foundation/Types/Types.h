#ifndef _APP_TYPES_H_
#define _APP_TYPES_H_

#include "Externals/etl/include/etl/enum_type.h"
#include <stdint.h>

struct FourCC {
  // While the names of the FourCC codes can be changed, their values CANNOT.
  // Values are used as is in save files, so any changes would cause save files
  // to break.
  enum enum_type {
    InstrumentCommandArpeggiator = 0,          // ARPG
    InstrumentCommandCrush = 2,                // CRSH dup
    InstrumentCommandDelay = 4,                // DLAY
    InstrumentCommandFilterCut = 20,           // FCUT
    InstrumentCommandLowPassFilter = 22,       // FLTR
    InstrumentCommandFilterResonance = 25,     // FRES
    InstrumentCommandGateOff = 92,             // GTOF
    InstrumentCommandGroove = 26,              // GROV
    InstrumentCommandHop = 27,                 // HOP
    InstrumentCommandRetrigger = 52,           // RTRG
    InstrumentCommandInstrumentRetrigger = 29, // IRTG
    InstrumentCommandKill = 30,                // KILL
    InstrumentCommandLegato = 31,              // LEGA
    InstrumentCommandLoopOfset = 36,           // LPOF
    InstrumentCommandMidiCC = 38,              // MDCC
    InstrumentCommandMidiPC = 39,              // MDPG,
    InstrumentCommandPan = 42,                 // PAN
    InstrumentCommandPitchFineTune = 44,       // PFIN
    InstrumentCommandPlayOfset = 46,           // PLOF
    InstrumentCommandPitchSlide = 48,          // PTCH
    InstrumentCommandStop = 55,                // STOP
    InstrumentCommandTable = 58,               // TABL dup
    InstrumentCommandTempo = 62,               // TMPO
    InstrumentCommandVelocity = 66,            // VELM
    InstrumentCommandVolume = 69,              // VOLM dup
    InstrumentCommandNone = 45,                // ----

    SampleInstrumentCrushVolume = 3,
    SampleInstrumentVolume = 69, // dup
    SampleInstrumentCrush = 2,   // dup
    SampleInstrumentSample = 54,
    SampleInstrumentInterpolation = 28,
    SampleInstrumentDownsample = 5,
    SampleInstrumentRootNote = 51,
    SampleInstrumentFineTune = 24,
    SampleInstrumentPan = 43,
    SampleInstrumentFilterCutOff = 22,
    SampleInstrumentFilterResonance = 25,
    SampleInstrumentFilterType = 23,
    SampleInstrumentFilterMode = 21,
    SampleInstrumentStart = 56,
    SampleInstrumentLoopMode = 34,
    SampleInstrumentLoopStart = 37,
    SampleInstrumentEnd = 6,
    SampleInstrumentTable = 58,           // dup
    SampleInstrumentTableAutomation = 60, // dup

    MacroInstrumentShape = 93,
    MacroInstrmentTimbre = 94,
    MacroInstrumentColor = 95,
    MacroInstrumentAttack = 96,
    MacroInstrumentDecay = 97,
    MacroInstrumentSignature = 98,

    MidiInstrumentChannel = 1,
    MidiInstrumentNoteLength = 32,
    MidiInstrumentVolume = 69,
    MidiInstrumentTable = 58,           // dup
    MidiInstrumentTableAutomation = 60, // dup

    SIDInstrument1Waveform = 72,
    SIDInstrument2Waveform = 73,
    SIDInstrument3Waveform = 74,
    SIDInstrument1FilterCut = 79,
    SIDInstrument2FilterCut = 83,
    SIDInstrument3FilterCut = 87,
    SIDInstrument1FilterResonance = 80,
    SIDInstrument2FilterResonance = 84,
    SIDInstrument3FilterResonance = 88,
    SIDInstrument1FilterMode = 81,
    SIDInstrument2FilterMode = 85,
    SIDInstrument3FilterMode = 89,
    SIDInstrument1Volume = 82,
    SIDInstrument2Volume = 86,
    SIDInstrument3Volume = 90,
    SIDInstrumentPulseWidth = 71,
    SIDInstrumentVSync = 75,
    SIDInstrumentRingModulator = 76,
    SIDInstrumentADSR = 77,
    SIDInstrumentFilterOn = 78,
    SIDInstrumentVoice3Off = 91,
    SIDInstrumentTable = 58,           // dup
    SIDInstrumentTableAutomation = 60, // dup

    ServicePersistency = 57,

    TrigTempoTap = 65,
    TrigSeqQueueRow = 64,
    TrigVolumeIncrease = 68,
    TrigVolumeDecrease = 67,
    TrigEventEnter = 7,
    TrigEventEdit = 8,
    TrigEventLeft = 10,
    TrigEventRight = 13,
    TrigEventUp = 15,
    TrigEventDown = 9,
    TrigEventAlt = 11,
    TrigEventNav = 12,
    TrigEventPlay = 14,

    VarTempo = 62,
    VarMasterVolume = 41,
    VarWrap = 70,
    VarTranspose = 63,
    VarScale = 16,
    VarProjectName = 99,
    VarMidiDevice = 40,
    VarLineOut = 17,
    VarFGColor = 103,
    VarBGColor = 104,
    VarHI1Color = 105,
    VarHI2Color = 106,
    VarConsoleColor = 107,
    VarCursorColor = 108,
    VarInfoColor = 109,
    VarWarnColor = 110,
    VarErrorColor = 111,
    VarMidiSync = 112,

    VarInstrumentType = 113,

    ActionTempoChanged = 61,
    ActionPurge = 49,
    ActionPurgeInstrument = 47,
    ActionProjectRename = 102,
    ActionLoad = 35,
    ActionSave = 53,
    ActionNewProject = 101,
    ActionRandomName = 100,
    ActionBootSelect = 18,
    ActionEdit = 59,

    // Free: 19, 33
    Default = 255, // "    "
  };
  ETL_DECLARE_ENUM_TYPE(FourCC, char)
  // Not all enums need reflection. Only cases where we need reflection is the
  // FourCC codes that need to be converted to text in order to display on
  // screen
  ETL_ENUM_TYPE(InstrumentCommandArpeggiator, "ARP")
  ETL_ENUM_TYPE(InstrumentCommandCrush, "CSH")
  ETL_ENUM_TYPE(InstrumentCommandKill, "KIL")
  ETL_ENUM_TYPE(InstrumentCommandLoopOfset, "LOF")
  ETL_ENUM_TYPE(InstrumentCommandVelocity, "VEL")
  ETL_ENUM_TYPE(InstrumentCommandVolume, "VOL")
  ETL_ENUM_TYPE(InstrumentCommandPitchSlide, "PSL")
  ETL_ENUM_TYPE(InstrumentCommandHop, "HOP")
  ETL_ENUM_TYPE(InstrumentCommandLegato, "LEG")
  ETL_ENUM_TYPE(InstrumentCommandRetrigger, "RTG")
  ETL_ENUM_TYPE(InstrumentCommandTempo, "TPO")
  ETL_ENUM_TYPE(InstrumentCommandMidiCC, "MCC")
  ETL_ENUM_TYPE(InstrumentCommandMidiPC, "MPC")
  ETL_ENUM_TYPE(InstrumentCommandPlayOfset, "POF")
  ETL_ENUM_TYPE(InstrumentCommandLowPassFilter, "FLT")
  ETL_ENUM_TYPE(InstrumentCommandTable, "TBL")
  ETL_ENUM_TYPE(InstrumentCommandFilterCut, "FCT")
  ETL_ENUM_TYPE(InstrumentCommandFilterResonance, "FRS")
  ETL_ENUM_TYPE(InstrumentCommandPan, "PAN")
  ETL_ENUM_TYPE(InstrumentCommandGateOff, "GOF")
  ETL_ENUM_TYPE(InstrumentCommandGroove, "GRV")
  ETL_ENUM_TYPE(InstrumentCommandStop, "STP")
  ETL_ENUM_TYPE(InstrumentCommandNone, "---")
  ETL_ENUM_TYPE(InstrumentCommandPitchFineTune, "PFT")
  ETL_ENUM_TYPE(InstrumentCommandDelay, "DLY")
  ETL_ENUM_TYPE(InstrumentCommandInstrumentRetrigger, "IRT")

  ETL_ENUM_TYPE(Default, "   ")
  ETL_END_ENUM_TYPE
};

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

#endif
