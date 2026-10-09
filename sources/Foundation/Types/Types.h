/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _APP_TYPES_H_
#define _APP_TYPES_H_

#include "Externals/etl/include/etl/enum_type.h"
#include <stdint.h>

struct TrackerCommand {
  // Command values are persisted in save files and must not change.
  enum enum_type : uint8_t {
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
    InstrumentCommandChance = 188, // CHN
    InstrumentCommandRandom = 189, // RND

    Default = 255,
  };
  ETL_DECLARE_ENUM_TYPE(TrackerCommand, uint8_t)
  ETL_ENUM_TYPE(InstrumentCommandArpeggiator, "ARP")
  ETL_ENUM_TYPE(InstrumentCommandChance, "CHN")
  ETL_ENUM_TYPE(InstrumentCommandCrush, "CSH")
  ETL_ENUM_TYPE(InstrumentCommandKill, "KIL")
  ETL_ENUM_TYPE(InstrumentCommandLoopOfset, "LOF")
  ETL_ENUM_TYPE(InstrumentCommandVelocity, "VEL")
  ETL_ENUM_TYPE(InstrumentCommandVolume, "VOL")
  ETL_ENUM_TYPE(InstrumentCommandPitchSlide, "PSL")
  ETL_ENUM_TYPE(InstrumentCommandRandom, "RND")
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

  ETL_ENUM_TYPE(Default, "   ")
  ETL_END_ENUM_TYPE
};

struct FourCC {
  // Runtime IDs are local to the firmware; variables are saved by name.
  enum enum_type : uint16_t {
    Default,

    SampleInstrumentCrushVolume,
    SampleInstrumentVolume,
    SampleInstrumentCrush,
    SampleInstrumentSample,
    SampleInstrumentInterpolation,
    SampleInstrumentDownsample,
    SampleInstrumentRootNote,
    SampleInstrumentFineTune,
    SampleInstrumentPan,
    SampleInstrumentFilterCutOff,
    SampleInstrumentFilterResonance,
    SampleInstrumentFilterType,
    SampleInstrumentFilterMode,
    SampleInstrumentStart,
    SampleInstrumentLoopMode,
    SampleInstrumentLoopStart,
    SampleInstrumentEnd,
    SampleInstrumentTable,
    SampleInstrumentTableAutomation,

    MacroInstrumentShape,
    MacroInstrmentTimbre,
    MacroInstrumentColor,
    MacroInstrumentAttack,
    MacroInstrumentDecay,
    MacroInstrumentSignature,

    MidiInstrumentChannel,
    MidiInstrumentNoteLength,
    MidiInstrumentVolume,
    MidiInstrumentTable,
    MidiInstrumentTableAutomation,
    MidiInstrumentName,
    MidiInstrumentProgram,

    SIDInstrumentWaveform,
    SIDInstrument1FilterCut,
    SIDInstrument2FilterCut,
    SIDInstrument3FilterCut,
    SIDInstrument1FilterResonance,
    SIDInstrument2FilterResonance,
    SIDInstrument3FilterResonance,
    SIDInstrument1FilterMode,
    SIDInstrument2FilterMode,
    SIDInstrument3FilterMode,
    SIDInstrument1Volume,
    SIDInstrument2Volume,
    SIDInstrument3Volume,
    SIDInstrumentPulseWidth,
    SIDInstrumentVSync,
    SIDInstrumentRingModulator,
    SIDInstrumentADSR,
    SIDInstrumentFilterOn,
    SIDInstrumentVoice3Off,
    SIDInstrumentTable,
    SIDInstrumentTableAutomation,
    SIDInstrumentOSCNumber,

    OPALInstrumentChannel,
    OPALInstrumentAlgorithm,
    OPALInstrumentFeedback,
    OPALInstrumentDeepTremeloVibrato,

    OPALInstrumentOp1Level,
    OPALInstrumentOp1Multiplier,
    OPALInstrumentOp1KeyScaleLevel,
    OPALInstrumentOp1ADSR,
    OPALInstrumentOp1WaveShape,
    OPALInstrumentOp1TremVibSusKSR,

    OPALInstrumentOp2Level,
    OPALInstrumentOp2Multiplier,
    OPALInstrumentOp2KeyScaleLevel,
    OPALInstrumentOp2ADSR,
    OPALInstrumentOp2WaveShape,
    OPALInstrumentOp2TremVibSusKSR,

    ServicePersistency,

    TrigTempoTap,
    TrigSeqQueueRow,
    TrigVolumeIncrease,
    TrigVolumeDecrease,
    TrigEventEnter,
    TrigEventEdit,
    TrigEventLeft,
    TrigEventRight,
    TrigEventUp,
    TrigEventDown,
    TrigEventAlt,
    TrigEventNav,
    TrigEventPlay,

    VarTempo,
    VarMasterVolume,
    VarPreviewVolume,
    VarWrap,
    VarTranspose,
    VarScale,
    VarScaleRoot,
    VarProjectName,
    VarMidiDevice,
    VarLineOut,
    VarFGColor,
    VarBGColor,
    VarHI1Color,
    VarHI2Color,
    VarConsoleColor,
    VarCursorColor,
    VarInfoColor,
    VarWarnColor,
    VarErrorColor,
    VarAccentColor,
    VarAccentAltColor,
    VarEmphasisColor,
    VarReserved1Color,
    VarReserved2Color,
    VarReserved3Color,
    VarReserved4Color,
    VarMidiSync,
    VarMidiClockSync,
    VarRemoteUI,
    VarUIFont,

    VarChannel1Volume,
    VarChannel2Volume,
    VarChannel3Volume,
    VarChannel4Volume,
    VarChannel5Volume,
    VarChannel6Volume,
    VarChannel7Volume,
    VarChannel8Volume,
    VarThemeName, // Variable for storing the current theme name

    VarInstrumentType,

    ActionTempoChanged,
    ActionPurge,
    ActionPurgeInstrument,
    ActionProjectRename,
    ActionBrowse,
    ActionSave,
    ActionLoadAndSave,
    ActionCancel,
    ActionNewProject,
    ActionRandomName,
    ActionBootSelect,
    ActionEdit,
    ActionExport,
    ActionImport,
    ActionOK,
    InstrumentName,
    ActionRenderMixdown,
    ActionRenderStems,
    ActionShowTheme,
    ActionThemeName,
    SampleInstrumentSlices,
    VarBacklightLevel,
    ActionShowSampleEditor,
    ActionShowSampleSlices,
    VarRecordSource,
    VarSampleEditStart,
    VarSampleEditEnd,
    VarSampleEditOperation,
    VarRecordLineGain,
    VarRecordMicGain,
    VarOutputVolume,
    VarImportResampler,
    ActionAutoSlice,
    ActionShowRecordView,
    ActionRebuildSampleCache,
  };
  ETL_DECLARE_ENUM_TYPE(FourCC, uint16_t)
  // Reflect the IDs used for display and variable persistence.
  ETL_ENUM_TYPE(VarLineOut, "LINEOUT")
  ETL_ENUM_TYPE(VarMidiDevice, "MIDIDEVICE")
  ETL_ENUM_TYPE(VarMidiSync, "MIDISYNC")
  ETL_ENUM_TYPE(VarMidiClockSync, "MIDICLOCKSYNC")
  ETL_ENUM_TYPE(VarRemoteUI, "REMOTEUI")
  ETL_ENUM_TYPE(VarUIFont, "UIFONT")
  ETL_ENUM_TYPE(VarThemeName, "THEMENAME")
  ETL_ENUM_TYPE(VarScaleRoot, "scaleroot")
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
  ETL_ENUM_TYPE(SIDInstrumentWaveform, "VWF")
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
  ETL_ENUM_TYPE(VarChannel1Volume, "channel1vol")
  ETL_ENUM_TYPE(VarChannel2Volume, "channel2vol")
  ETL_ENUM_TYPE(VarChannel3Volume, "channel3vol")
  ETL_ENUM_TYPE(VarChannel4Volume, "channel4vol")
  ETL_ENUM_TYPE(VarChannel5Volume, "channel5vol")
  ETL_ENUM_TYPE(VarChannel6Volume, "channel6vol")
  ETL_ENUM_TYPE(VarChannel7Volume, "channel7vol")
  ETL_ENUM_TYPE(VarChannel8Volume, "channel8vol")

  ETL_ENUM_TYPE(ActionEdit, "edit")
  ETL_ENUM_TYPE(ActionExport, "export")
  ETL_ENUM_TYPE(ActionImport, "import")
  ETL_ENUM_TYPE(ActionThemeName, "themename")
  ETL_ENUM_TYPE(VarBacklightLevel, "backlightLevel")
  ETL_ENUM_TYPE(VarRecordSource, "recordsource")
  ETL_ENUM_TYPE(VarRecordLineGain, "recordlinegain")
  ETL_ENUM_TYPE(VarRecordMicGain, "recordmicgain")
  ETL_ENUM_TYPE(VarOutputVolume, "outputvolume")
  ETL_ENUM_TYPE(VarImportResampler, "IMPORTRESAMP")

  ETL_ENUM_TYPE(Default, "   ")
  ETL_END_ENUM_TYPE
};

static_assert(sizeof(TrackerCommand) == sizeof(uint8_t),
              "Tracker commands are persisted as one byte");
static_assert(sizeof(FourCC) == sizeof(uint16_t),
              "Runtime IDs are expected to be two bytes");
static_assert(FourCC::VarChannel8Volume == FourCC::VarChannel1Volume + 7,
              "Channel volume IDs must remain contiguous");

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef uint32_t stereosample;

#endif
