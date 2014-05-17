#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

PLATFORM:=  RASPI

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

NDSDIRS	:=    . \
				../sources/Adapters/NDS/FileSystem \
				../sources/Adapters/NDS/System \
				../sources/Adapters/NDS/Main \
				../sources/Adapters/Dummy/Midi \
				../sources/Adapters/NDS/Sound \
				../sources/Adapters/NDS/UI
				


DEBDIRS	:=			../sources/Adapters/Unix/FileSystem \
				../sources/Adapters/DEB/Main \
				../sources/Adapters/Unix/Process \
				../sources/System/Process \
				../sources/Externals/RTAudio \
				../sources/Externals/RtMidi \
				../sources/Adapters/DEB/System \
				../sources/Adapters/Jack/Audio\
				../sources/Adapters/Jack/Midi\
				../sources/Adapters/Jack/Client\
				../sources/Adapters/Jack/Midi\
				../sources/Adapters/RTAudio\
				../sources/Adapters/RTMidi\
				../sources/Adapters/SDL/Timer\
				../sources/Adapters/SDL/GUI 

BEAGLEBOARDDIRS :=                      ../sources/Adapters/Unix/FileSystem \
                                ../sources/Adapters/DEB/Main \
                                ../sources/Adapters/Unix/Process \
                                ../sources/System/Process \
                                ../sources/Externals/RTAudio \
                                ../sources/Adapters/DEB/System \
                                ../sources/Adapters/Dummy/Midi \
                                ../sources/Adapters/Jack/Audio\
                                ../sources/Adapters/Jack/Midi\
                                ../sources/Adapters/Jack/Client\
                                ../sources/Adapters/Jack/Midi\
                                ../sources/Adapters/RTAudio\
                                ../sources/Adapters/SDL/Timer\
                                ../sources/Adapters/SDL/GUI

RASPIDIRS :=                      ../sources/Adapters/Unix/FileSystem \
                                ../sources/Adapters/DEB/Main\
                                ../sources/Adapters/Unix/Process\
                                ../sources/System/Process\
                                ../sources/Externals/RTAudio\
                                ../sources/Externals/RtMidi\
                                ../sources/Adapters/DEB/System\
                                ../sources/Adapters/Jack/Audio\
                                ../sources/Adapters/Jack/Midi\
                                ../sources/Adapters/Jack/Client\
                                ../sources/Adapters/Jack/Midi\
                                ../sources/Adapters/RTAudio\
                                ../sources/Adapters/RTMidi\
                                ../sources/Adapters/SDL/Timer\
                                ../sources/Adapters/SDL/GUI

GP2XDIRS	:=    . \
				../sources/Adapters/GP2X/Main \
				../sources/Adapters/GP2X/FileSystem \
				../sources/System/Process \
				../sources/Adapters/Unix/Process \
				../sources/Adapters/GP2X/System \
				../sources/Adapters/GP2X/Midi \
				../sources/Adapters/GP2X/Audio \
				../sources/Adapters/SDL/GUI\
				../sources/Adapters/SDL/Timer 

CAANOODIRS	:=    . \
				../sources/Adapters/CAANOO/Main \
				../sources/Adapters/Unix/FileSystem \
				../sources/System/Process \
				../sources/Adapters/Unix/Process \
				../sources/Adapters/CAANOO/System \
				../sources/Adapters/CAANOO/Midi \
				../sources/Adapters/CAANOO/Audio \
				../sources/Adapters/SDL/GUI\
				../sources/Adapters/SDL/Timer 

DINGOODIRS      :=    . \
				../sources/Adapters/DINGOO/Main \
				../sources/Adapters/Unix/FileSystem \
				../sources/System/Process \
				../sources/Adapters/Unix/Process \
				../sources/Adapters/DINGOO/System \
				../sources/Adapters/DINGOO/Midi \
				../sources/Adapters/Dummy/Midi \
				../sources/Adapters/DINGOO/Audio \
				../sources/Adapters/SDL/Audio \
				../sources/Adapters/SDL/Process \
				../sources/Adapters/SDL/GUI \
				../sources/Adapters/SDL/Timer

PSPDIRS	:=    . \
				../sources/Adapters/PSP/Main \
				../sources/Adapters/PSP/FileSystem \
				../sources/System/Process \
				../sources/Adapters/PSP/System \
				../sources/Adapters/Dummy/Midi \
				../sources/Adapters/SDL/Process \
				../sources/Adapters/SDL/Audio\
				../sources/Adapters/SDL/GUI\
				../sources/Adapters/SDL/Timer 

GP32DIRS	:=	.\
				../sources/Adapters/GP32FileSystem \
				../sources/Adapters/GP32Midi \
				../sources/Adapters/GP32/Main \
				../sources/Adapters/GP32System \
				../sources/Adapters/GP32Timer \
				../sources/Adapters/GP32UI

W32DIRS     :=  .\
				../sources/Adapters/W32FileSystem \
				../sources/Adapters/W32Midi \
				../sources/Adapters/W32Sound \
				../sources/Adapters/WSDLSystem \
				../sources/Adapters/WSDLUI \
				../sources/Adapters/SDL/Input \
				../sources/Adapters/W32/Main \
				../sources/Adapters/W32/Process \
				../sources/Adapters/W32/Audio \
				../sources/System/Process

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
				../sources/Externals/Compression \
				../sources/Externals/Soundfont \
				../sources/Externals/TinyXML

#---------------------------------------------------------------------------------
# files definition
#---------------------------------------------------------------------------------

DINGOOFILES             :=  GPSDLMain.o \
					UnixFileSystem.o \
					DummyMidi.o \
					DINGOOAudio.o \
					Process.o \
					UnixProcess.o \
					SDLAudioDriver.o \
					DINGOOAudioDriver.o \
					DINGOOSystem.o \
					DINGOOEventQueue.o \
					SDLProcess.o \
					SDLTimer.o \
					GUIFactory.o \
					SDLEventManager.o \
					SDLGUIWindowImp.o

PSPFILES		:=	PSPmain.o  \
					PSPFileSystem.o \
					Process.o \
					DummyMidi.o \
					GUIFactory.o \
					SDLGUIWindowImp.o \
					SDLEventManager.o \
					SDLAudioDriver.o \
					SDLAudio.o \
					SDLProcess.o \
					SDLTimer.o \
					PSPSystem.o  
					
DEBFILES		:=	UnixFileSystem.o \
					DEBmain.o \
					DEBSystem.o \
					GUIFactory.o \
					SDLGUIWindowImp.o \
					SDLEventManager.o \
					Process.o \
					UnixProcess.o \
					RTAudioDriver.o \
					RTAudioStub.o \
					RtAudio.o\
					RtMidi.o\
					RTMidiService.o\
					RTMidiOutDevice.o\
                                        RTMidiInDevice.o\
					SDLTimer.o \
					JackAudioDriver.o \
					JackClient.o \
					JackAudio.o 

BEAGLEBOARDFILES                :=      UnixFileSystem.o \
                                        DEBmain.o \
                                        DEBSystem.o \
                                        GUIFactory.o \
                                        SDLGUIWindowImp.o \
                                        SDLEventManager.o \
                                        Process.o \
                                        UnixProcess.o \
                                        DummyMidi.o \
                                        RTAudioDriver.o \
                                        RTAudioStub.o \
                                        RtAudio.o\
                                        SDLTimer.o 

RASPIFILES                :=      UnixFileSystem.o \
                                        DEBmain.o \
                                        DEBSystem.o \
                                        GUIFactory.o \
                                        SDLGUIWindowImp.o \
                                        SDLEventManager.o \
                                        Process.o \
                                        UnixProcess.o \
                                        RTAudioDriver.o \
                                        RTAudioStub.o \
                                        RtAudio.o\
                                        RtMidi.o\
                                        RTMidiService.o\
                                        RTMidiOutDevice.o\
                                        RTMidiInDevice.o\
                                        SDLTimer.o \
                                        JackAudioDriver.o \
                                        JackClient.o \
                                        JackAudio.o 

NDSFILES		:=	NDSmain.o \
					NDSFileSystem.o \
					DummyMidi.o \
					NDSSound.o \
					NDSSystem.o \
					GUIFactory.o NDSGUIWindowImp.o

W32FILES        :=	WSDLmain.o \
					W32FileSystem.o \
					W32Midi.o \
					W32AudioDriver.o \
					WSDLSystem.o \
					SDLInput.o  \
					Process.o \
					W32Process.o  \
					W32Audio.o  \
					GUIFactory.o WSDLGUIWindowImp.o

GP2XFILES		:=	GPSDLMain.o \
					GP2XFileSystem.o \
					GP2XMidiService.o \
					GP2XSerialMidiDevice.o \
					GP2XAudio.o \
					Process.o \
					UnixProcess.o \
					GP2XAudioDriver.o \
					GP2XSystem.o \
					GP2XEventQueue.o \
					gpmemcpy.o \
					gpmemset.o \
					SDLTimer.o \
					GUIFactory.o SDLEventManager.o SDLGUIWindowImp.o 

CAANOOFILES		:=	CAANOOSDLMain.o \
					UnixFileSystem.o \
					CAANOOMidiService.o \
					CAANOOSerialMidiDevice.o \
					CAANOOAudio.o \
					Process.o \
					UnixProcess.o \
					CAANOOAudioDriver.o \
					CAANOOSystem.o \
					CAANOOEventQueue.o \
					gpmemcpy.o \
					gpmemset.o \
					SDLTimer.o \
					GUIFactory.o SDLEventManager.o SDLGUIWindowImp.o 

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
					MixerView.o NullView.o \
					SongView.o ChainView.o PhraseView.o ViewData.o ConsoleView.o ProjectView.o InstrumentView.o UIController.o \
					View.o ModalView.o FieldView.o UIField.o UIIntField.o \
					UIIntVarOffField.o UIIntVarField.o ViewEvent.o I_Action.o\
					UITempoField.o UIActionField.o \
					MessageBox.o \
					GrooveView.o UINoteVarField.o UIBigHexVarField.o \
					SRPUpdaters.o UIStaticField.o \
					Song.o Chain.o Phrase.o Project.o \
					char.o n_assert.o fixed.o wildcard.o \
					SyncMaster.o TablePlayback.o Player.o \
					Table.o TableView.o\
					InstrumentBank.o WavFileWriter.o WavFile.o MidiInstrument.o Filters.o SampleVariable.o SampleInstrument.o SamplePool.o CommandList.o \
					PersistencyService.o Persistent.o \
					Observable.o SingletonRegistry.o \
					Audio.o AudioMixer.o AudioOutDriver.o AudioDriver.o \
					AudioOut.o \
					DummyAudioOut.o PlayerChannel.o AudioFileStreamer.o \
					MixBus.o \
					MixerService.o PlayerMixer.o \
					Service.o ServiceRegistry.o SubService.o \
					Variable.o VariableContainer.o WatchedVariable.o \
					SoundFontPreset.o SoundFontManager.o SoundFontSample.o \
					ENAB.o HYDRA.o OMEGA.o RIFF.o SFDETECT.o SFLOOKUP.o SFNAV.o SFREADER.o WIN_MEM.o \
					HexBuffers.o lz.o \
					tinyxmlparser.o tinyxml.o tinyxmlerror.o tinystr.o Tiny2NosStub.o \

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
        
