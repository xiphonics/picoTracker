# What is LittleGPTracker?
[*LittleGPTracker*](http://www.10pm.org/nostromo/lgpt/) *(a.k.a piggy tracker) is a sample based tracker originally programed for the gamepark handhelds, but now also available for Microsoft Windows and Mac OSX. It implements a user interface similar to the refined track-by-joypad software* [*littlesounddj*](http://www.littlesounddj.com/)*. Piggy currently supports 8 monophonic 8Bit/16Bit/44.1Khz stereo channels as well as 16 channel midi output. Piggy is currently under development and the latest versions of the program (along with unstable ghetto builds) can be found at* [*http://gorehole.org/lgptWiki/*](http://gorehole.org/lgptWiki/)*.*
You are reading a reference manual for those who are comfortable with LSDj or trackers in general. An alternative introduction to LittleGPTracker is our very own [quick_start_guide](http://wiki.littlegptracker.com/doku.php?id=lgpt:quick_start_guide). If you have a question this manual does not answer, please ask on the [lgpt mailing list](http://groups.yahoo.com/groups/littlegptracker/) or in #hexawe on efnet. Don't forget to check out the advanced [tips_tricks](http://wiki.littlegptracker.com/doku.php?id=lgpt:tips_tricks)
If you want to grab some sounds to get off the ground quickly, or want to hear and see what other people have been doing with piggy tracker, download some of the .zip archives from [http://hexawe.net](http://hexawe.net/)!

# Files
## Installation

Grab the installation package for your platform from the [download page](http://www.10pm.org/nostromo/lgpt/download.php). Extract the files from the archive & copy it where you like. after that you can run the executable type for your system, located in the lgpt root or in the /bin folder. linux users may need to set the binary as executable (chmod a+x lgpt.deb-exe).

## Directory Structure
- **bin**
  - **lgpt.exe / lgpt.deb-exe / lgpt.app** … desktop executable file
  - **SDL.dll** … dependency
  - **stderr.txt** … debugging output file (created at startup and removed upon exit)
  - **stdout.txt** … debugging output file (created at startup and removed upon exit)
- **docs**
  - **readme.txt** … credits and links to piggy resources
  - **revision.txt** … version history, describes how new features work if they're not included in the manual yet
- **lgpt10k** … Project folder to the author's entry for the [10k compo](http://www.10pm.org/nostromo/lgpt/10k.php) (M-.-n : Tardline) Note: all projects start with lgpt*
  - **Samples** … each project loads wav files from it's own Samples directory
  - **lgptsav.dat** … all projects store their data in xml files called lgptsav.dat
- **lgptNew** … another piggy project, following the lgpt* naming convention for projects
  - **Samples** … when you start a new project you have to make a Samples directory and load it up (NOTE: Since version X.XX??? this is unnecessary as LGPT creates this folder when a new project is created in the opening selector screen)
  - **lgptsav.dat** … this file doesn't exist for new projects but will be created by lgpt when you first save
- **lgpt.fxe** … gp32 executable
- **lgpt.gpe** … gp2x executable
- **SampleLib** … Place your samples here

Getting lgpt to run depends on your platform:

- **gp32:** place LGPT.fxe, lgptNew, and lgpt10k in \GPMM
- **gp2x/caanoo:** place LGPT.gpe, lgptNew, and lgpt10k in the same folder (anywhere on your SD card)
- **win/mac/*nix :** everything should be in the right spot, just run the executable in \install\bin\ (for fullscreen run “lgpt.exe -fullscreen”)
## Importing Samples

Piggy doesn't save samples inside a project file like traditional trackers. Instead, when you save, piggy will create project data as /lgptRoot/lgptProject/lgptsav.dat and store the samples you are using in /lgptRoot/lgptProject/samples. This means that instead of copying/sharing a single module file, you need to distribute the whole lgptProject directory.

Samples can get inside that /lgptRoot/lgptProject/samples folder two ways: using the sample import dialog inside of piggy, or copying the wav files in by hand (via whatever os you are running).

The sample import dialog is the easiest way to preview and add files to your lgptProject. Any samples which you import from that screen will be copied from the samplelib to /lgptRoot/lgptProject/samples when you save the project.
If you like to do things by hand you can make a new lgptProject folder and a samples folder in there. Prior to loading a project for the first time you can copy some wav files into /lgptRoot/lgptProject/samples and piggy will automatically assign each wav to a new instrument.
After that you can copy additional wavs to the lgptRoot/lgptProject/samples directory. The next time you load the project piggy will have them in it's sample list but the wavs will not yet be associated with any instruments.

**Use 8 or 16 Bit wav files, any sampling frequency, mono or stereo**. 8bit samples are converted to 16bit at load time for compatibility with the engine (you can save space in storage but not in RAM).

**Piggy now supports .sf2 Soundfonts. You must add these by hand to your SAMPLES directory, use PROGRAM CHANGE commands to load different patches. Loop points are automatically loaded, but you'll need to make VOLM setting to adjust decay.**

## Multiple Projects

The Piggy supports multiple projects! Just create multiple directories in the root folder (where lgptNew and lgpt10k were found). Examples: “lgptProject1”, “lgpt*Author*Name”, “lgptSomethingElse”, “lgptWhatever”, etc. Each project directory must contain its own samples which must be stored in a sub-directory called “samples”. Lgpt will prompt you to choose one of the projects found in the root (lgpt- directories) when starting up.
Important Points to Remember:

- Project directories **must** start with “lgpt”.
- Project directories **must** go in the root folder.
- You can reuse a previously made lgptsav.dat file.
- Lgpt will list all the projects available on startup.
- The lgptsav.dat file is created automatically the project's folder using the piggy's save function (see Controls & Moves). You should not create tah file manually.
- Save often :)
## config.xml

The config file is used to tweak certain global parameters of the application. It is not mandatory to have one but it will allow you to change key mapping, color scheme, key repeat setting and other.
For more details check out the [config](http://wiki.littlegptracker.com/doku.php?id=lgpt:config) page

# Controls & Moves

If you want to change the default mapping in LGPT, you will want to edit the confiq.xml. More information can be found on this on the [config](http://wiki.littlegptracker.com/doku.php?id=lgpt:config)
If you want to augment the mapping controls to add a usb footpedal, another joystick, macros using additional buttons on your device, or even control of piggy via midi commands, check out [mapping](http://wiki.littlegptracker.com/doku.php?id=lgpt:mapping)

## Key Mapping

**Gamepark keys**

- DPAD (or references to LEFT, RIGHT, UP, DOWN): Common Joystick directions.
- A, B: As themselves.
- SELECT, & START: Starring as themselves.
- RT: Right trigger (shoulder button).
- LT: Left trigger (shoulder button).

*GP2X Specific notice*: The A & B buttons are “inverted” (when compared to GP32 and LSDJ on Gameboy). This can be modified in the [config](http://wiki.littlegptracker.com/doku.php?id=lgpt:config).

The moves are close to what you would pull in lsdj, but shoulder buttons replace the lsdj SELECT key. Most of the time, the equivalent of SELECT is the shoulder button opposite the other buttons you're pushing. For example SELECT+B is LT+B, SELECT+RIGHT is RT+RIGHT.

**Windows key mapping**

These are the defaults, which can be over-ridden in the config.xml file:

- A: A on US keyboard.
- B: S on US keyboard.
- START: Space.
- LT: Right Ctrl Key.
- RT: Left Ctrl Key.
- LT+RT+SELECT Combo: Esc Key (Go to Project Selection Screen/Quit LGPT).

Note: CTRL Key mappings of RT and LT are inverted. Since the keyboard's Arrow Keys are on the opposite side compared to the GamePark layout, so are LT and RT.

## Basic Editing & Navigation
- ARROWs: In screen navigation.
- A: Insert Chain/Phrase/Note.
- A,A: Insert next unused Chain/Phrase/Instrument.
- LT+(B,A): Clone. This will overwrite the current Highlighted Item with a copy of itself using the next unused Item available.
- B+A: Cuts the current Highlighted Item .
- A+ARROWS: Updates Highlighted Item value.
  - A+UP/DOWN: +/- 0x10.
  - A+RIGHT/LEFT: +/- 1.
- B+ARROWS: Rapid Navigation.
  - B+UP/DOWN: Page up/down in Song Screen, Next/Previous Phrase in Current Chain in Phrase Screen. Navigation +/- 0x10 in Instrument/Table Screen.
  - B+LEFT/RIGHT: Next/Previous Channel in Chain/Phrase Screen. Navigation +/- 1 in Instrument/Table Screen. Switch between Song and Live Modes in Song Screen.
- RT+ARROWS: Navigate between the Screens.
- LT+UP/DOWN: Jump up/down to next populated row after a blank row (great for live mode entire row queuing!)

## Selections

a few ways to make a selection:

- LT+B: Starts selection mode with only the data at the cursor selected
- LT+B+B: Starts selection mode with the cursor's row selected
- LT+B+B+B: Starts selection mode with the entire screen selected

once a selection is started you can do a few more things:

- ARROWS: will make an existing selection bigger or smaller
- B: copy selection to buffer, or
- LT+A: cut current selection

And then:

- LT+A: paste the clipboard content at current location

## Playback Modes and Controls

There are two modes for playback, Song and Live. The controls in each mode differ slightly.
You can switch between the modes by hitting B+LEFT/RIGHT in the Song screen.

**Song Mode**

    START:

In the Song Screen: Starts/Stops song playback from the Highlighted Row. If one of the Row's Channel is marked `--`, LGPT will ignore that Channel entirely for the rest of the playback.
In the Chain Screen: Starts/Stops playback from the Highlighted Step, soloing the Edited Chain.
In the Phrase Screen: Starts/Stops playback from Step `00`, soloing the Edited Phrase.

    RT+START:

Starts song playback from the Edited Row in all screens.

**Live Mode**

    START:

In the Song Screen: Queues from the Highlighted Channel Step.
In the Chain Screen: Queues the Highlighted Phrase Step.
In the Phrase Screen: Queues the Edited Chain Step from `00`.

- The Queued Item will be played as soon as the playing Chain on the Edited Channel reaches its last Step.
- If there is no playing Chain Step on the Edited Channel, the next Song Chain that reaches its last Step will trigger playback.
- Queued Items are shown with a blinking `>`.

    START[again]:

Pressing `START` a second time will queue the Item using Immediate Mode.

- The Queued Item will be played as soon as the playing Phrase on its Channel reaches the last Step.
- Immediate Mode Queued Items are shown with a fast blinking `>`.

    LT+START:

In the Song Screen: Queues all Channel Steps on the current Row.

- The Queued Items will be played as soon as the playing Chain on their Channel reaches its last Step.
- Queued Items are shown with a blinking `>`.

    LT+START[again]:

Pressing `START` a second time will queue the Items using Immediate Mode.

- The Queued Items will be played as soon as the playing Phrase on the their Channel reaches its last Step.
- Immediate Mode Queued Items are shown with a fast blinking `>`.

    RT+START:

In all Screens: Queues the Edited Channel Step to be stopped.

- The Queued Channel will be stopped as soon as its playing Chain reaches the last Step.
- Queued Items are shown with a blinking `_`.

    RT+START[again]:

Pressing `START` a second time will queue the Items using Immediate Mode.

- The Queued Channel will be stopped as soon as its playing Phrase reaches the last Step.
- Immediate Mode Queued Items are shown with a fast blinking `_`.

## Muting

- RT+B: Toggles mute/unmute of cursor channel
  - if RT is released before B, channel stays mutes
  - if B is released before RT, channel goes back to original state
- RT+A: Solo cursor channel
  - if RT is released before A, channel stays solo'ed
  - if A is released before RT, all channel go back to original state
- LT+RT: restore full playback on all channels
- RT+A,RT+B can be used in conjunction with selections. If a selection is present the toggle mute/solo action is done on all channels present in the selection

## System Operations
- (DEFUNCT →)Return to the song selection screen by pressing L_SHOULDER+R_SHOULDER+SELECT (Esc on windows). Only works when piggy is not playing. WARNING: all data is currently forgotten when resetting to the song selection screen.

# Screens

## Screen Map

```
o888                          o8
 888    oooooooo8 ooooooooo  o888oo
 888  888    88o   888    888 888
 888   888oo888o   888    888 888
o888o 888     888  888ooo88    888o
       888ooo888  o888 *screenmap by Sm0hm*

   _-------_                       _-------_
  |         |                     |         |
  |    P    |                     |    G    |
  |         |                     |         |
   project--                       groove---
       ^                               ^
       v                               v
   _-------_       _-------_       _-------_       _-------_
  |         |     |         |     |         |     |         |
  |    S    | <-> |    C    | <-> |    P    | <-> |    I    |
  |         |     |         |     |         |     |         |
   song-----       chain----       phrase---       instrument
                                       ^               ^
                                       v               v
                                   _-------_       _-------_
                                  |         |     |         |
                                  |    T    | <-> |    T    |
                                  |         |     |         |
                                   table----       table----
```

To move from one screen to the other, press the RTrigger combined with the direction. To get to the chain screen, you need to have your cursor on a chain in the song. To get to the phrase screen, you need to have your cursor on a pattern in the chain screen.

## Selector Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140398472_selector_1.1f.png)

- All the folders you have named as lgptWhatever will show up here.
- Up and Down to select a project, hit A to load the project.
- B+Up/Down will go Up/Down a whole page.

## Project Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140419022_project_1.1f.png)

- **Tempo:**: Can be set between 60bpm [0x3c] and 400bpm [0x190]. Resolution aligned to LSDJ.
- **Master:** Main volume goes from 10% to 200%. Piggy is loud now!
- **Transpose:** Live transposition of every triggered instruments.
- **Compact Sequencer:** Free all unused chain/phrases.
- **Compact Instruments:** All unused instruments get their sample set to (null), old parameter settings stick. A dialog offers to remove unused samples.
- **Load Song:** Brings you back to the Selector Screen.
- **Save Song:** Save the work you do, and save frequently! The cursor will disappear while the data is being written.
- **midi:** Lets you chose external MIDI device.

## Song Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140431434_song_1.1fb.png)

- Songs are made up of chains. Each hex number on the song screen grid is a chain. You can navigate to any of those chains by putting your cursor on one and hitting RT+R.
- The top of the song screen displays if you are in SONG or LIVE mode (toggle with B+L or B+R) and the name of the project you are editing (choose a new one with LT+RT+SELECT, but save before you go there because there is no turning back).
- On the bottom of the screen is the play-time visualizer. Each box correlates with the above columns in the song grid. When a sample is triggered in one of the columns, the note value of that trigger is displayed in the play-time visualizer.
- On the right of the song screen we have some more play-time statistics. First there is the clip indicator which displays “—-” when your volume levels are ok and “clip” when you've cleared the headroom and are chopping off the top of your samples. Next is the CPU load indicator as a percentage value. After that is the number “100” that will flash when your battery is getting low (gp2x firmware 2.X and lower only!). The bottom row of the play-time statistics is the amount of time which has passed since you pressed start.
- You can make a new chain by hitting A,A on a blank space in the song screen grid.
- You can clone a chain by highlighting it with the cursor and pressing LT+(B,A).
- You can copy a chain or selection of chains by highlighting and pressing B.
- You can cut or delete a chain or selection of chains by highlighting and pressing B+A.
- Make a big selection by pressing LT+B, then DPAD around to highlight.
- Press LT+A to paste.
- You can jump the cursor to the next/previous chain in a column by pressing LT+DOWN/UP

## Chain Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140447738_chain_1.1f.png)

- The three columns of the phrase screen, from left to right: (red) row counter, phrase list, and transpose.
- Chains are made up of phrases. Each hex number in the second column represents a phrase.
- You can navigate to any of those phrases by putting your cursor on one and hitting RT+R.
- The rightmost column is transpose. If you use very high numbers like FF the phrase on that row will transpose down. Low numbers like 04 will cause the phrase on that row to transpose up.
- you can jump to previous / next chain on the row with with B+LEFT/RIGHT
- You can make a new phrase by hitting A,A on a blank space in the Chain screen.
- You can clone a phrase by highlighting it with the cursor and pressing LT+(B,A).
- You can copy a phrase/transposition/selection by highlighting and pressing B.
- You can cut or delete a phrase/transposition/selection by highlighting and pressing B+A.
- Make a big selection by pressing LT+B, then DPAD around to highlight.
- Press LT+A to paste.

## Phrase Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140464770_phrase_1.1f.png)

- The top of the phrase screen displays the .wav file loaded into the instrument under your cursor. The left of the phrase screen is the play-time visualizer.
- The seven columns of the phrase screen, from left to right: (red) row counter, note trigger, instrument selector, effect one, parameters for effect one, effect two, parameters for effect two.
- You can clone instruments and tables in the phrase screen: LT+(B,A) as usual. You can get a new table or instrument: A,A.
- If you copy/cut anything in the phrase screen, pasting will always put the data back in the same column (regardless if you've moved the cursor to another column). so effects in column one are always pasted back there, and you can't accidentally paste a note into the effect column, etc.
- you can clone a MIDI instrument in the phrase screen by pressing L_TRIGGER+(B,A) on instrument number in phrase screen. Properly saved & restored. You can't delete them once cloned (can't revert them to sample type).
- If no instrument is set when triggering a new note, tables are not stopped, running commands are not stopped and phase of oscillator instrument is not reset (allowing for clickless transition)

## Instrument Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140478226_instrument_1.1f.png)

- **sample:** selects the .wav file to associate with the instrument. you can select the same sample in more than one instrument. if you tap A,A here it will take you to the Sample Import Screen (which lets you load new .WAV into your project).
- **volume:**
- **pan:** pans the instrument left or right (0x7F is center)
- **root note**
- **drive:** This is a volume modification before crush, and the instruments volume is after the crush.
- **crush:** decrease bit resolution
- **downsample:** decrease the bit rate, get those low frequency aliasing whines. each increase in this value will downsample the original sample by a factor of 2
- **cutoff:** filter cutoff frequency
- **reso:** filter resonance frequency
- **type:** this is where it gets a little trickier. The filter now supports continuous change from low pass to high pass. set type to 00 for low pazz. FF for hi-pass and 7f for Band pass (or is it notch? n0s must check). all intermediate values morph in between them.
- **dist:** filter distortion. for the moment we have none & scream. i'm planning on maybe add a third choice that would make the filter behave a little better when resonance is set very high in the old/default mode
- **fb tune:** length of the feedback delay line
- **fb mix:** how much of the feedback is pushed back in the circuit

The feedback pickup is at the very end of the chain, after the filter has been processed. It makes it very depending on volume, filter settings, etc… but makes it more organic too. After pickup, it stays in a delay line of variable length (through the tune parameter) and is mixed back with the current sample value, before crush, filter and so on. I've found it very nice to fatten oscillators and give some kind of a nice warm-distorted bass sounds.
The feedbacktune parameter works a little differently in regular (one shot/loop) and in oscillator mode:
In regular modes, under 0x80, it delays the feedback pickup of a number of samples equivalent to the parameter. Over 0x80, the feedback line length is multiplied by 10, giving it more a sort of 'predelay'-ish character that works very well with short decayed sounds
In oscillator modes, under 0x80 the feedback of specified length is added to the oscillator. Over 0x80 it is removed from the oscillator, making it a lot wilder.

- **interpolation:** Interpolation mode ('linear'/'none'): selects which interpolation mode is used when in between samples. linear interpols linearly while none takes the nearest neighbor. Use none when playing samples at low range to add some typical overtones.
- **loop mode:** selects the looping mode.
  - none will play sample from zero to end.
  - loop will start at zero and loop from loopstart to end.
  - looper sync will automatically tune a loop so that it plays exactly 16 bars. Use the root note to play twice faster/slower
  - oscillator is a special mode where the loop selection (from loopstart to end) is taken as oscillator data and automatically tuned. Experiment with different settings, do not forget 'root note' is your friend to tune the oscillator back in a useful range
- **start:** start point of the sample regardless of if loop is enabled; in hex
- **loop Start:** start point of the sample when loop is enabled; in hex
- **loop End:** end point of the sample; in hex. You can play samples backwards by setting the end value lower than the start!
- **automation:** If On, the table play arrows will advance one row every time the instrument is triggered, and execute only the commands on the new rows. If this is Off, table behavior is normal (play arrows cruise around real fast).
- **table:** Select a table the instrument will always run. Clone a table here: LT+(B,A). Make a new table here: A,A.

## Sample Import Screen

**NOTE: THE NEWEST GHETTOS HAVE A NEW SAMPLE BROWSER. THIS APPLIES TO PRE 1.0 VERSIONS.**
accessible by hitting A,A on the “sample” parameter in the Instrument Screen.
The samples of the library have to be located in a folder samplelib at the same level as the song folders (lgpt-xxxx). you can either put your samples in that directory or in sub-directories, allowing you to have some basic way of sorting the library. For example:

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140496106_sample_import_1.1f.png)


When entering the import screen, the current folder is the library root folder “samplelib”. All sample in that folder are listed.
Use U/D to select a sample and 'A' to load it
B+L/R to rotates between all sub directories.
In the latest ghetto, hitting 'A,A“ will bring up a sample loader pop-up screen, use the cursor to select directories and samples, and chose “listen” to play the sample, “import” to add it to your project, or “exit” to return the instrument screen.

## Midi Instrument Screen
![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140537386_midi_1.1f.png)


Midi can be enabled on many platforms through simple software configs in your OS, or through the construction of platform specific hardware. More info about this can be found [**here**](http://wiki.littlegptracker.com/doku.php?id=lgpt:midi)

A midi instrument has the following settings:

- **Channel** - This can be set 0x80 to 0x8F which is midi channel 1-16 respectively
- **Volume** - The volume any NOTE ON will be sent to your device. FF=127, 00=00
- **Length** - Sets note gate length in number of ticks.
- **Automation** - On, the table play arrows will advance one row every time the instrument is triggered, and execute only the commands on the new rows. If this is Off, table behavior is normal (play arrows cruise around real fast).
- **Table**- As above, select a table the instrument will always run. Clone a table here: LT+(B,A). Make a new table here: A,A.

## Table Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140549562_table_1.1f.png)

- Hopping to self in table holds the step for x ticks. For example

05 HOP 1005
will hold step 5 for 10 ticks. Interesting for example after a VOLM command to allow complex envelopes:
00 VOLM 0400 ; starts short volume decay to zero 01 HOP 1010 ; holds enough ticks for VOLM to complete 02 VOLM 0560 ; raise volume to 60 03 HOP 6003 ; hold for a long time to allow VOLM to ; complete and hold volume to 60

- After Hopping count is reached, the table pointer moves directly to the line after

the HOP rather than staying on the HOP line for one tick. Makes more sense.

## Groove Screen

![](https://d2mxuefqeaa7sj.cloudfront.net/s_B9C92C3440E8671360862F10CAB0FE70873BFE49CFB51CA246C781C17506C258_1522140567776_groove_1.1f.png)

Groove screen is located on top of the phrase screen. Groove lets you rock some swing to bust funky beats.
To grasp groove you must first you must know what a tick is, and a tick is simply a constant measurement of time. By default, every row of a phrase (aka step) gets 6 ticks, that's why you see two sixes when you go to the groove screen. But say you change the first 6 to a 1. Now, every other step lasts for 1 tick (which is shorter than 6 ticks).
To get a handle on this, try a groove of 1/F and watch the play arrow in a phrase. You'll notice the play arrow hanging around on the odd steps longer than the even ones. Then try a groove of 1/1/F and go back to the phrase screen. You'll see the play arrow hanging on every third step. Once you grasp how the groove screen effects the play arrow, you can move to more subtle values or keep the beat way freaked out!
In the groove screen there is no copy paste (yet) but the following action exists:

- A: Adds a new step if not existing
- A+L/R: modifies current step value
- B+L/R/U/D: modifies current edited groove
- B+A: Clear current step

the GROV command (only active in the phrase screen) select the current groove

# Commands

There can be two commands on every row of the phrase screen. Commands which effect instruments can be run on any step of the instruments playback, including the step where the instrument is triggered.
in vol; pitch and kill but the definition of the “time' is slightly different for all command…

## ARPG abcd

**cycle through relative pitches a, b, c, and d (starting with original pitch, then up a semitones, b semitones and so forth). The cycle loops if there's only zero's past a given post**
Examples:
ARPG 3000: loops between original pitch and +3 semitones
ARPG 4050: loops between original pitch, +4 semitones, +0 semitones, + 5 semitones

- speed of arpeggiator is constant and can not be changed
## CRSH aabb

**aa = pre crush drive (from 1 to 0xFF, 00 is no change) & bb = crush setting (from 0 to 0xF, 0x0 is 1 bit, 0xF is 16bit )**

## DLAY --bb

**Delays the note to be played by bb tics**

## FCUT aabb

**adjust the filter cutoff to bb at speed aa**

- FCUT 0080 will instantly set the filter cutoff to 50%
- FCUT 1000 will close the filter entirely at speed 10
## FLTR aabb

**lowpass filter, set absolute frequency value for cutoff aa & resonance bb**

- FLTR 00FF is un-adultered sound
## FRES aabb

**adjust the filter resonance to bb at speed aa**

- FRES 08FF will raise the resonance to screeching at speed 08
## FBMX aabb

**go to feedback mix xxbb at aaxx speed**

## FBTN aabb

**go to feedback tune xxbb at aaxx speed**

## HOP aabb

**play position will jump to the next phrase in a chain, jumping directly at position bb in the phrase.**

- hop is instant: instrument triggers and commands on the same row will be run.
- no effect on instruments
- in [TABLES](file:///C:/Users/naray/Downloads/lgpt%20reference_manual.html#table_screen), cursor position will jump to row bb aa times, then pass thru the hop command and continue thru the rest of the table

## IRTG aabb

IRTG stands for Instrument Retrigger and will retrigger the current instrument. It gives table the ability to work as progammable phrases that then can be triggered simply by changing tables.
IRTG –bb will retrigger the current instrument transposed by bb semi-tones. Note that each IRTG transposition is cumulatively added. So a table with
IRTG 0001
will keep going a semi tone up. Great for dubby echoes :)
The retriggered instrument is NOT reset (as if you enter a note with no instrument number). The table (obviously) will continue to run and all running variable (filter,etc) won't be reset.
This system is also pretty useful to implement temporary non 4/4 signature without having to switch grooves, since you have the ability to re-trigger the instrument at tick resolution
don't forget trying to combine it with complex hop structure !

## KILL --bb

**instrument will stop playing after aa ticks.**

## LEGA aabb

**performs an exponential pitch slide from previous note value to pitch bb at speed aa.**

- 00 is the fastest speed for aa (instant, useless)
- bb values are relative: 00-7F are up, 80-FF are down, expressed in semi-tones
- if LEGA is put on a row where a note is present and the pitch offset is 0 (e.g. `C4 I3 LEGA 1000`) the slide will occur automatically from previous note to the current one at the given speed.
- If an instrument is not triggered on the same row as LEGA, the command will re-trigger the previous instrument (unless the previous instrument is still playing).
- LEGA does exponential pitch change (i;e. it goes at same speed through all octaves) while PITCH is linear

## LPOF aaaa

**LooP OFset: Shift both the loop start & loop end values aaaa digits**

- LPOF 0001 adds one to both values, LFOF FFFF removes one (so values > 0x800 moves the loop backward)
- reset everytime you start a new note (same as volume, pitch)
- LPOF is absolute
- you can't trigger a note with the LPOF, it has to be executed after a sample is playing
- every time you trigger a sample LPOF is set back to the instrument parameters
## MDCC aabb

**Sends a MIDI “continuous control” message. aa is the control number and bb is the value. It will be sent on the MIDI channel of the currently running instrument.**

## MDPG --bb

sends a program change command on the current channel. 0000 is program change 1

## PAN aabb

**PAN aabb: where bb is the pan destination and aa is the speed to get there**

## PFIN aabb

**PitchFINetune: where bb is the width and aa is the speed to get there**

- Tunes the root note one semitone up (01-80) or down (FF-81)
- 00 in bb returns the note to the root center
- 00 is the fastest speed for aa

## PLOF aabb

**PLayOFfset virtually cuts any sample in 256 chunks. jump absolutely to chunk aa or relatively move forward/back bb chunks.**

## PTCH aabb

- PTCH is also time for the first two byte nibble
- PITCH is linear pitch change

## RTRG aabb

**retrigger the sound by looping the from current play position over a certain amount of ticks.**

- aa allows to move the loop forward of aa ticks each time the loop has been done (loop offest per retrigger)
- bb is the number of ticks used for the looping (speed of retrigger effect)

RTRG 0001: loop one tick from current play position
RTRG 0102: loop of two ticks but move the loop one tick every loop
RTRG 0101: does not do anything because after looping one tick, you move forward one tick and therefore go back to the current position :)

## TABL --bb

**triggers table bb**

## TMPO --bb

**sets the tempo to hex value –bb.**

- TMPO 0000 is safe and doesn't effect the tempo at all.
- TMPO 003C (60bpm) is the lowest acceptable value and TMPO 0190 (400bpm) is the highest acceptable value

## VOLM aabb

**starting from the instrument's volume setting, approach volume bb at speed aa. 00 is the lowest volume and 00 is the fastest speed (instant).**

- to achieve sounds that grow in volume, make an instrument with volume 0 and then apply the VOLM command

# Rendering

Some people exploit the analog gap between their device's headphone output and whatever they are recording with. Alternately, you can start piggy in rendering mode so it will output 16bit, 44100Hz .WAV files.
Please note that RENDER mode is not intended to be functional on the GP2X Builds.
The following values can set for RENDER in the config.xml:

- Standard mode: audio is played; no render.
- FILE: File rendering: Full speed (no audio) rendering of the stereo mixdown.
- FILESPLIT: File split rendering: Full speed (no audio) rendering of each channel separately.
- FILERT: Real Time file rendering: Renders the mixdown to file WHILE playing audio. This allow to render live mode tweaks directly.
- FILESPLITRT: Real Time file split: same except all channels are rendered separately.

Here is an example of the proper XML syntax: (See [The config.xml setup guide](http://wiki.littlegptracker.com/doku.php?id=lgpt:config))

```
<RENDER value = "FILERT" />
```

Remember, any of the config.xml parameters can be specified to lgpt on the command line in this fashion:

```
lgpt -RENDER=FILE
```