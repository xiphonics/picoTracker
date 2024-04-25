#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

PLATFORM:=  LINUX

include $(PWD)/Makefile.$(PLATFORM)

#---------------------------------------------------------------------------------
# options for FXE
#---------------------------------------------------------------------------------

AUTHOR	:=	n0s
TITLE	:=	lgpt
NAME    :=  LittleGPTracker
ICON    :=  lgpt_icon.bmp


#---------------------------------------------------------------------------------
# Directory definitions
#---------------------------------------------------------------------------------

OSXDIRS := .\
				../sources/Adapters/OSX/OSXMain \
				../sources/Adapters/Unix/Process \
				../sources/System/Process \
				../sources/Adapters/SDL/Audio \
				../sources/Adapters/OSX/OSXSystem \
				../sources/Adapters/SDL/Timer\
				../sources/Adapters/SDL/GUI \
				../sources/Adapters/Unix/FileSystem \
				../sources/Adapters/Dummy/Midi \

LINUXDIRS := .\
				../sources/Adapters/OSX/OSXMain \
				../sources/Adapters/Unix/Process \
				../sources/System/Process \
				../sources/Adapters/SDL/Audio \
				../sources/Adapters/OSX/OSXSystem \
				../sources/Adapters/SDL/Timer\
				../sources/Adapters/SDL/GUI \
				../sources/Adapters/Unix/FileSystem \
				../sources/Adapters/Dummy/Midi \

COMMONDIRS	:=	../sources/System/Console \
				../sources/System/io \
				../sources/Services/Midi \
				../sources/Services/Time \
				../sources/Services/Controllers \
				../sources/System/FileSystem \
				../sources/System/Timer \
				../sources/System/Errors \
				../sources/UIFramework/Framework \
				../sources/UIFramework/BasicDatas \
				../sources/UIFramework/SimpleBaseClasses \
				../sources/UIFramework/Interfaces \
				../sources/Application \
				../sources/Application/Controllers \
				../sources/Application/Commands \
				../sources/Application/Audio \
				../sources/Application/Views \
				../sources/Application/Views/ModalDialogs \
				../sources/Application/Views/BaseClasses \
				../sources/Application/Model \
				../sources/Application/Utils \
				../sources/Application/Player \
				../sources/Application/Mixer \
				../sources/Application/Instruments \
				../sources/Application/Persistency \
				../sources/Foundation \
				../sources/Services/Audio \
				../sources/Foundation/Services \
				../sources/Foundation/Variables \
				../sources/Externals/Soundfont \
				../sources/Externals/TinyXML \
				../sources/Externals/yxml \

#---------------------------------------------------------------------------------
# files definition
#---------------------------------------------------------------------------------

OSXFILES := UnixFileSystem.o \
	OSXmain.o \
	SDLMain.o \
	OSXSystem.o \
	SDLGUIWindowImp.o \
	SDLEventManager.o \
	SDLAudioDriver.o \
	SDLAudio.o \
  DummyMidi.o \
	SDLTimer.o \
  Process.o \
	UnixProcess.o \
	GUIFactory.o \

COMMONFILES		:=	Logger.o Trace.o Result.o Status.o Config.o \
					Timer.o FileSystem.o \
					SysMutex.o TimeService.o \
					MidiOutDevice.o MidiInDevice.o MidiService.o Groove.o \
					MidiChannel.o MidiInMerger.o \
					GUIEvent.o GUIRect.o \
					EventManager.o GUIWindow.o \
					Channel.o Mixer.o \
					ControlRoom.o \
					MultiChannelAdapter.o \
					CommandDispatcher.o \
					ApplicationCommandDispatcher.o \
					EventDispatcher.o \
					ControllerService.o \
					ButtonControllerSource.o \
					KeyboardControllerSource.o \
					JoystickControllerSource.o \
					HatControllerSource.o \
					ControllerSource.o \
					ControlNode.o \
					I_GUIWindowImp.o \
					Application.o AppWindow.o SelectProjectDialog.o \
					NewProjectDialog.o \
					ImportSampleDialog.o \
					NullView.o \
					SongView.o ChainView.o PhraseView.o ViewData.o ConsoleView.o ProjectView.o InstrumentView.o UIController.o \
					View.o ModalView.o FieldView.o UIField.o UIIntField.o \
					UIIntVarOffField.o UIIntVarField.o ViewEvent.o I_Action.o\
					UITempoField.o UIActionField.o \
					MessageBox.o \
					GrooveView.o UINoteVarField.o UIBigHexVarField.o \
					SRPUpdaters.o UIStaticField.o \
					Song.o Chain.o Phrase.o Project.o Scale.o \
					char.o n_assert.o fixed.o wildcard.o \
					SyncMaster.o TablePlayback.o Player.o \
					Table.o TableView.o\
					InstrumentBank.o WavFileWriter.o WavFile.o MidiInstrument.o Filters.o SampleVariable.o SampleInstrument.o SamplePool.o CommandList.o \
					PersistencyService.o Persistent.o PersistencyDocument.o \
					Observable.o SingletonRegistry.o \
					Audio.o AudioMixer.o AudioOutDriver.o AudioDriver.o \
					AudioOut.o \
					DummyAudioOut.o PlayerChannel.o AudioFileStreamer.o \
					MixBus.o \
					MixerService.o PlayerMixer.o \
					Service.o ServiceRegistry.o SubService.o \
					Variable.o VariableContainer.o WatchedVariable.o \
					HexBuffers.o \
		      		yxml.o \

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------

TARGET		:=	$(TITLE)
BUILD		:=	build$(PLATFORM)
SOURCES		:=	$($(PLATFORM)DIRS) $(COMMONDIRS)

INCLUDES	:=


#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)
#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------



#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
export LD	:=	$(CXX)

export OFILES	:=	$(COMMONFILES) $($(PLATFORM)FILES) 
export OBJS := $(OFILES)
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(PWD)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(OUTPUT).$(EXTENSION) $(BUILD) *.elf *.gxb *.fxe


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

ifeq ($(PLATFORM),PSP)
include $(PSPSDK)/lib/build.mak
endif

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

all: $(OUTPUT).$(EXTENSION)

$(OUTPUT).exe   :   $(OFILES)
	@echo "linking result.."
	@$(LD) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)

$(OUTPUT).fxe	:	$(OUTPUT).gxb

$(OUTPUT).gxb	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)

$(OUTPUT).deb-exe	:	$(OFILES)
	$(LD) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)

$(OUTPUT).beagle-exe	:	$(OFILES)
	$(LD) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)


-include $(DEPENDS)


#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------


deploy:
	../scripts/deployW32.sh 
	../scripts/deployGP2X.sh 
	../scripts/deployGP32.sh 
	../scripts/deployOSX.sh 
        
