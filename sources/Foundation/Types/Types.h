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
    InstrumentCommandCrush = 2,                // CRSH
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
    InstrumentCommandMidiPC = 39,              // MDPG
    InstrumentCommandPan = 42,                 // PAN
    InstrumentCommandPitchFineTune = 44,       // PFIN
    InstrumentCommandPlayOfset = 46,           // PLOF
    InstrumentCommandPitchSlide = 48,          // PTCH
    InstrumentCommandStop = 55,                // STOP
    InstrumentCommandTable = 58,               // TABL
    InstrumentCommandTempo = 62,               // TMPO
    InstrumentCommandVelocity = 66,            // VELM
    InstrumentCommandVolume = 69,              // VOLM
    InstrumentCommandNone = 45,                // ----
    InstrumentCommandMidiChord = 143,

    SampleInstrumentCrushVolume = 3,
    SampleInstrumentVolume = 19,
    SampleInstrumentCrush = 114,
    SampleInstrumentSample = 54,
    SampleInstrumentInterpolation = 28,
    SampleInstrumentDownsample = 5,
    SampleInstrumentRootNote = 51,
    SampleInstrumentFineTune = 24,
    SampleInstrumentPan = 43,
    SampleInstrumentFilterCutOff = 115,
    SampleInstrumentFilterResonance = 116,
    SampleInstrumentFilterType = 23,
    SampleInstrumentFilterMode = 21,
    SampleInstrumentStart = 56,
    SampleInstrumentLoopMode = 34,
    SampleInstrumentLoopStart = 37,
    SampleInstrumentEnd = 6,
    SampleInstrumentTable = 117,
    SampleInstrumentTableAutomation = 60,

    MacroInstrumentShape = 93,
    MacroInstrmentTimbre = 94,
    MacroInstrumentColor = 95,
    MacroInstrumentAttack = 96,
    MacroInstrumentDecay = 97,
    MacroInstrumentSignature = 98,

    MidiInstrumentChannel = 1,
    MidiInstrumentNoteLength = 32,
    MidiInstrumentVolume = 118,
    MidiInstrumentTable = 119,
    MidiInstrumentTableAutomation = 120,
    MidiInstrumentName = 144,
    MidiInstrumentProgram = 160,

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
    SIDInstrumentTable = 121,
    SIDInstrumentTableAutomation = 122,
    SIDInstrumentOSCNumber = 142,

    OPALInstrumentChannel = 123,
    OPALInstrumentAlgorithm = 124,
    OPALInstrumentFeedback = 125,
    OPALInstrumentDeepTremeloVibrato = 126,

    OPALInstrumentOp1Level = 127,
    OPALInstrumentOp1Multiplier = 128,
    OPALInstrumentOp1KeyScaleLevel = 130,
    OPALInstrumentOp1ADSR = 131,
    OPALInstrumentOp1WaveShape = 132,
    OPALInstrumentOp1TremVibSusKSR = 133,

    OPALInstrumentOp2Level = 134,
    OPALInstrumentOp2Multiplier = 135,
    OPALInstrumentOp2KeyScaleLevel = 136,
    OPALInstrumentOp2ADSR = 137,
    OPALInstrumentOp2WaveShape = 138,
    OPALInstrumentOp2TremVibSusKSR = 139,

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

    VarTempo = 33,
    VarMasterVolume = 41,
    VarPreviewVolume = 161,
    VarWrap = 70,
    VarTranspose = 63,
    VarScale = 16,
    VarScaleRoot = 162,
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
    VarAccentColor = 152,
    VarAccentAltColor = 153,
    VarEmphasisColor = 154,
    VarReserved1Color = 155,
    VarReserved2Color = 156,
    VarReserved3Color = 157,
    VarReserved4Color = 158,
    VarMidiSync = 112,
    VarMidiClockSync = 151,
    VarRemoteUI = 140,
    VarUIFont = 141,
    // 142 is taken for SIDInstrumentOSCNumber
    // 143 is taken for InstrumentCommandMidiChord
    // 144 is taken for InstrumentMidiName
    // 145 is taken for ActionExport
    // 146 is taken for ActionImport
    // 147 is taken for ActionOK
    // 148 is taken for InstrumentName
    // 149 is taken for ActionRenderMixdown
    // 150 is taken for ActionRenderStems
    // 151 is taken for VarMidiClockSync
    // 152 is taken for VarPlayColor
    // 153 is taken for VarMuteColor
    // 154 is taken for VarSongViewFEColor
    // 155 is taken for VarSongView00Color
    // 156 is taken for VarRowColor
    // 157 is taken for VarRow2Color
    // 158 is taken for VarMajorBeatColor
    // 159 is taken for ActionShowTheme
    // 160 is taken for MidiInstrumentProgram
    // 161 is taken for VarScaleRoot
    // 162 is taken for VarPreviewVolume

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
    ActionExport = 145,
    ActionImport = 146,
    ActionOK = 147,
    InstrumentName = 148,
    ActionRenderMixdown = 149,
    ActionRenderStems = 150,
    ActionShowTheme = 159,

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
  ETL_ENUM_TYPE(InstrumentCommandMidiChord, "MCH")

  ETL_ENUM_TYPE(VarLineOut, "LINEOUT")
  ETL_ENUM_TYPE(VarMidiDevice, "MIDIDEVICE")
  ETL_ENUM_TYPE(VarMidiSync, "MIDISYNC")
  ETL_ENUM_TYPE(VarMidiClockSync, "MIDICLOCKSYNC")
  ETL_ENUM_TYPE(VarRemoteUI, "REMOTEUI")
  ETL_ENUM_TYPE(VarUIFont, "UIFONT")
  ETL_ENUM_TYPE(VarScaleRoot, "SCALEROOT")
  ETL_ENUM_TYPE(MacroInstrumentShape, "shape")
  ETL_ENUM_TYPE(MacroInstrmentTimbre, "timbre")
  ETL_ENUM_TYPE(MacroInstrumentColor, "color")
  ETL_ENUM_TYPE(MacroInstrumentAttack, "Attack")
  ETL_ENUM_TYPE(MacroInstrumentDecay, "Decay")
  ETL_ENUM_TYPE(MacroInstrumentSignature, "Signature")
  ETL_ENUM_TYPE(SampleInstrumentSample, "sample")
  ETL_ENUM_TYPE(SampleInstrumentVolume, "volume")
  ETL_ENUM_TYPE(SampleInstrumentInterpolation, "interpol")
  ETL_ENUM_TYPE(SampleInstrumentCrush, "crush")
  ETL_ENUM_TYPE(SampleInstrumentCrushVolume, "crushdrive")
  ETL_ENUM_TYPE(SampleInstrumentDownsample, "downsample")
  ETL_ENUM_TYPE(SampleInstrumentRootNote, "root note")
  ETL_ENUM_TYPE(SampleInstrumentFineTune, "fine tune")
  ETL_ENUM_TYPE(SampleInstrumentPan, "pan")
  ETL_ENUM_TYPE(SampleInstrumentFilterCutOff, "filter cut")
  ETL_ENUM_TYPE(SampleInstrumentFilterResonance, "filter res")
  ETL_ENUM_TYPE(SampleInstrumentFilterType, "filter type")
  ETL_ENUM_TYPE(SampleInstrumentFilterMode, "filter mode")
  ETL_ENUM_TYPE(SampleInstrumentStart, "start")
  ETL_ENUM_TYPE(SampleInstrumentLoopMode, "loopmode")
  ETL_ENUM_TYPE(SampleInstrumentLoopStart, "loopstart")
  ETL_ENUM_TYPE(SampleInstrumentEnd, "end")
  ETL_ENUM_TYPE(SampleInstrumentTable, "table")
  ETL_ENUM_TYPE(SampleInstrumentTableAutomation, "table automation")
  ETL_ENUM_TYPE(MidiInstrumentChannel, "channel")
  ETL_ENUM_TYPE(InstrumentName, "name")
  ETL_ENUM_TYPE(MidiInstrumentName, "midi name")
  ETL_ENUM_TYPE(MidiInstrumentNoteLength, "note length")
  ETL_ENUM_TYPE(MidiInstrumentVolume, "volume")
  ETL_ENUM_TYPE(MidiInstrumentTable, "table")
  ETL_ENUM_TYPE(MidiInstrumentTableAutomation, "table automation")
  ETL_ENUM_TYPE(MidiInstrumentProgram, "program")
  ETL_ENUM_TYPE(SIDInstrument1Waveform, "VWF1")
  ETL_ENUM_TYPE(SIDInstrument2Waveform, "VWF2")
  ETL_ENUM_TYPE(SIDInstrument1FilterCut, "FILTCUT1")
  ETL_ENUM_TYPE(SIDInstrument1FilterResonance, "RES1")
  ETL_ENUM_TYPE(SIDInstrument1FilterMode, "FMODE1")
  ETL_ENUM_TYPE(SIDInstrument1Volume, "DIP_VOLUME1")
  ETL_ENUM_TYPE(SIDInstrument2FilterCut, "FILTCUT2")
  ETL_ENUM_TYPE(SIDInstrument2FilterResonance, "RES2")
  ETL_ENUM_TYPE(SIDInstrument2FilterMode, "FMODE2")
  ETL_ENUM_TYPE(SIDInstrument2Volume, "DIP_VOLUME2")
  ETL_ENUM_TYPE(SIDInstrumentPulseWidth, "VPW")
  ETL_ENUM_TYPE(SIDInstrumentVSync, "VSYNC")
  ETL_ENUM_TYPE(SIDInstrumentRingModulator, "VRING")
  ETL_ENUM_TYPE(SIDInstrumentADSR, "VADSR")
  ETL_ENUM_TYPE(SIDInstrumentFilterOn, "VFON")
  ETL_ENUM_TYPE(SIDInstrumentTable, "table")
  ETL_ENUM_TYPE(SIDInstrumentTableAutomation, "table automation")
  ETL_ENUM_TYPE(SIDInstrumentOSCNumber, "OSCNUM")

  // channel variable not currently used by OPAL instruments but maybe in future
  ETL_ENUM_TYPE(OPALInstrumentChannel, "CHANNEL")
  ETL_ENUM_TYPE(OPALInstrumentAlgorithm, "ALGORITHM")
  ETL_ENUM_TYPE(OPALInstrumentFeedback, "FEEDBACK")
  ETL_ENUM_TYPE(OPALInstrumentDeepTremeloVibrato, "DEEPTREMELOVIBRATO")

  ETL_ENUM_TYPE(OPALInstrumentOp1Level, "OP1LEVEL")
  ETL_ENUM_TYPE(OPALInstrumentOp1Multiplier, "OP1MULTIPLIER")
  ETL_ENUM_TYPE(OPALInstrumentOp1KeyScaleLevel, "OP1KEYSCALELEVEL")
  ETL_ENUM_TYPE(OPALInstrumentOp1ADSR, "OP1ADSR")
  ETL_ENUM_TYPE(OPALInstrumentOp1WaveShape, "OP1WAVESHAPE")
  ETL_ENUM_TYPE(OPALInstrumentOp1TremVibSusKSR, "OP1TREMVIBSUSKSR")

  ETL_ENUM_TYPE(OPALInstrumentOp2Level, "OP2LEVEL")
  ETL_ENUM_TYPE(OPALInstrumentOp2Multiplier, "OP2MULTIPLIER")
  ETL_ENUM_TYPE(OPALInstrumentOp2KeyScaleLevel, "OP2KEYSCALELEVEL")
  ETL_ENUM_TYPE(OPALInstrumentOp2ADSR, "OP2ADSR")
  ETL_ENUM_TYPE(OPALInstrumentOp2WaveShape, "OP2WAVESHAPE")
  ETL_ENUM_TYPE(OPALInstrumentOp2TremVibSusKSR, "OP2TREMVIBSUSKSR")

  ETL_ENUM_TYPE(VarFGColor, "FOREGROUND")
  ETL_ENUM_TYPE(VarBGColor, "BACKGROUND")
  ETL_ENUM_TYPE(VarHI1Color, "HICOLOR1")
  ETL_ENUM_TYPE(VarHI2Color, "HICOLOR2")
  ETL_ENUM_TYPE(VarConsoleColor, "CONSOLECOLOR")
  ETL_ENUM_TYPE(VarCursorColor, "CURSORCOLOR")
  ETL_ENUM_TYPE(VarInfoColor, "INFOCOLOR")
  ETL_ENUM_TYPE(VarWarnColor, "WARNCOLOR")
  ETL_ENUM_TYPE(VarErrorColor, "ERRORCOLOR")
  ETL_ENUM_TYPE(VarAccentColor, "ACCENTCOLOR")
  ETL_ENUM_TYPE(VarAccentAltColor, "ACCENTALTCOLOR")
  ETL_ENUM_TYPE(VarEmphasisColor, "EMPHASISCOLOR")
  ETL_ENUM_TYPE(VarReserved1Color, "RESERVED1COLOR")
  ETL_ENUM_TYPE(VarReserved2Color, "RESERVED2COLOR")
  ETL_ENUM_TYPE(VarReserved3Color, "RESERVED3COLOR")
  ETL_ENUM_TYPE(VarReserved4Color, "RESERVED4COLOR")
  ETL_ENUM_TYPE(VarTempo, "tempo")
  ETL_ENUM_TYPE(VarMasterVolume, "master")
  ETL_ENUM_TYPE(VarPreviewVolume, "preview")
  ETL_ENUM_TYPE(VarWrap, "wrap")
  ETL_ENUM_TYPE(VarTranspose, "transpose")
  ETL_ENUM_TYPE(VarScale, "scale")
  ETL_ENUM_TYPE(VarProjectName, "projectname")
  ETL_ENUM_TYPE(VarInstrumentType, "INSTRUMENTTYPE")

  ETL_ENUM_TYPE(ActionEdit, "edit")
  ETL_ENUM_TYPE(ActionExport, "export")
  ETL_ENUM_TYPE(ActionImport, "import")

  ETL_ENUM_TYPE(Default, "   ")
  ETL_END_ENUM_TYPE
};

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef uint32_t stereosample;

#endif
