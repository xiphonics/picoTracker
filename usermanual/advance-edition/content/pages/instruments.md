---
title: Instruments
template: page
---

## Instrument Types

picoTracker supports several different types of instruments, each with its own unique capabilities and parameters. You can switch between instrument types using the **Type** field at the top of the instrument screen.

### Switching Instrument Types

1. Navigate to the instrument screen by pressing <span class="minikeys">NAV</span>+<span class="minikeys">RIGHT</span> from the phrase screen
2. By default the `NONE` instrument type is selected
3. Press <span class="minikeys">EDIT</span>+<span class="minikeys">LEFT</span> or <span class="minikeys">EDIT</span>+<span class="minikeys">RIGHT</span> to cycle through the available instrument types:
   - **SAMPLE**: The new, advanced sample instrument with integrated slicer and Voice screen
   - **ORIGINAL SAMPLE**: The legacy sampler instrument for backward compatibility and simplicity
   - **MIDI**: For controlling external MIDI devices
4. If you've made changes to the current instrument, you'll be asked to confirm before switching types
5. Note that you cannot change instrument types while playback is active

### Importing and Exporting Instruments

Once you've created an instrument, you can save it for use in other projects:

1. Make sure your instrument has a name set in the "name:" field
2. Select "Export" on the instrument screen
3. To import a previously saved instrument, select "Import"

## Sample Instrument

The Sample instrument is an advanced, flexible sampler that provides deep control over your sounds. It features an integrated waveform slicer on its main screen and a dedicated **Voice** screen for advanced sound shaping.

![screen capture of sample instrument](image/instrument-sample-screen-small.png)

### Main Parameters

- **sample:** Selects the .wav file to associate with this instrument. Tap <span class="minikeys">ENTER</span>,<span class="minikeys">ENTER</span> to go to the Sample Import Screen.
- **Waveform Display / Slicer:** Displays the sample waveform and active slices. You can directly edit and select slices here.
- **slices:** Shows the number of slices applied to the sample. Select `Adjust` to open the Sample Slices view (up to 64 slices).
- **level:** Overall output level of the instrument.
- **root note:** The base pitch of the sample.
- **table:** Select a table for the instrument to run.
- **automation:** If On, the table advances one row every time the instrument is triggered.
- **mode:** Selects the playback mode:
  - `oneshot`: Plays the sample from start to finish once.
  - `reverse`: Plays the sample backwards from end to start.
  - `loop`: Loops from loopstart to loopend.
  - `reverse loop`: Loops backwards from loopend to loopstart.
  - `loop pingpong`: Loops back and forth between loopstart and loopend.
  - `oscillator`: Treats the loop selection as a single-cycle oscillator and tunes it automatically.
  - `looper sync`: Automatically tunes the loop to play exactly 16 bars.
- **start:** Start point of the sample.
- **loop start:** Start point when looping is enabled.
- **loop end:** End point for playback or looping.

### Voice Screen

Press <span class="minikeys">NAV</span>+<span class="minikeys">RIGHT</span> from the Sample instrument screen to access the **Voice** screen. This screen provides advanced parameters for shaping the sound of the Sample instrument.

- **Trigger Length:** Controls the duration of the internal note trigger.
- **Amp Envelope:** A full Attack, Decay, Sustain, and Release (ADSR) envelope for controlling the instrument's volume over time.
- **Filter 1 (Bandpass):** A dedicated bandpass filter with Cutoff, Resonance, and Mix controls.
- **Filter 2 (Multimode):** A flexible multimode filter.
  - **Cutoff / Resonance:** Controls the filter frequency and peak.
  - **Type:** Morphs the filter from Lowpass (00) to Bandpass (7F) to Highpass (FF).
  - **Mix:** Blends the filtered signal with the dry signal.
- **Filter Envelope:** An ADSR envelope dedicated to Filter 2's cutoff frequency, with an adjustable **Amount** to control the depth of the modulation.
- **Insert Effects:** Two insert slots for adding effects directly to the voice.

  | Field | Full name | Values |
  | --- | --- | --- |
  | **FX1** / **FX2** | Insert effect slot 1 / 2 | `none`, `odrv`, `crsh`, `down`, `slew`, `rect` |
  | **Amt** | Amount | `00`-`FF` |
  | **Pos** | Position | `pre`, `post` |

  Available effects:
  - **none:** Bypasses the insert slot.
  - **odrv** (overdrive): Adds overdrive distortion to the signal.
  - **crsh** (bitcrush): Reduces the bit resolution of the audio, creating a lo-fi, granular texture.
  - **down** (downsample): Reduces the effective sample rate, introducing aliasing and low-frequency warble.
  - **slew:** Limits the maximum rate of change between consecutive samples, smoothing sharp transients. At low amounts the effect is subtle; at high amounts the signal ramps slowly, producing a warped, softened sound.
  - **rect** (rectify): Applies full-wave rectification, folding the negative portions of the waveform into the positive range. This introduces even harmonics and a frequency-doubling character. The amount controls a blend between the original and rectified signal.
- **LFO:** Two Low Frequency Oscillators, **LFO1** and **LFO2** are available for modulating almost every other voice parameter. As you select the target for each LFO the matching field will be displayed with a `1` or `2` symbol next to the fields label.

  | Field | Full name | Values |
  | --- | --- | --- |
  | **Tgt** | Target | `OFF`, `BASE`, `WDTH`, `PAN`, `FX1`, `FX2`, `FCOF`, `FRES`, `FENV`, `FATK`, `FDEC`, `FSUS`, `FREL`, `AMP`, `AATK`, `ADEC`, `ASUS`, `AREL` |
  | **Shp** | Shape | sine, triangle, square, ramp, random |
  | **Trg** | Trigger | `free`, `trig` |
  | **Typ** | Type | `spd`, `syn` |
  | **Spd** | Speed | `00`-`FF` in `spd` mode, or `4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`, `1/2T`, `1/4T`, `1/8T`, `1/16T` in `syn` mode |
  | **Lvl** | Level | `00`-`FF`, clamped to the selected target's range |

## Original Sample Instrument

This is the legacy sampler instrument, preserved for compatibility with older projects and for those who prefer its specific feature set.

![screen capture of original sample screen](image/instrument-original-sample-screen-small.png)

- **sample:** Selects the .wav file to associate with this instrument. You can use the same sample in more than one instrument. Tap <span class="minikeys">ENTER</span>,<span class="minikeys">ENTER</span> to go to the Sample Import Screen which lets you load new .wav files into your project, with the last imported sample selected as the sample assigned to this instrument [1]
- **volume:** Set the volume of the instrument
- **pan:** Pans the instrument left or right (0x7F is center)
- **root note:** The root note of the sample
- **detune:** Detune the sample by the number of semitones
- **drive:** This is a volume modification before crush, and the instruments volume is after the crush
- **crush:** Decreases the bit resolution
- **downsample:** Decreases the bit rate (eg. low frequency aliasing whines). Each increase in this value will downsample the original sample by a factor of 2
- **cutoff:** Set the Filter cutoff frequency
- **reso:** Set the Filter resonance frequency
- **type:** The filter supports continuous change from low pass to high pass. Set type to `00` for low pass. `FF` for high pass and `7F` for band pass. All intermediate values morph in between them
- **dist:** Set the filter distortion. Available values are `none` and `scream`

- **interpolation:** Interpolation mode ('linear'/'none'). Selects which interpolation mode is used when in between samples. `Linear` interpolates linearly while `none` takes the nearest neighbor. Use none when playing samples at low range to add some typical overtones. Note using linear interpolation currently adds significantly to the CPU load during playback.
- **loop mode:** selects the looping mode.
  - `none` will play sample from start to finish
  - `loop` will start at the start and loop from loopstart to end.
  - `looper sync` will automatically tune a loop so that it plays exactly 16 bars. Use the root note to play twice faster/slower
  - `oscillator` is a special mode where the loop selection (from loopstart to end) is taken as oscillator data and automatically tuned. Note that 'root note' can be used to tune the oscillator back in a useful range
- **start:** start point of the sample regardless of if loop is enabled (note value is in hex)
- **loop Start:** start point of the sample when loop is enabled (note value is in hex)
- **loop End:** end point of the sample (note value is in hex). You can play samples backwards by setting the end value lower than the start
- **slices:** Shows number of slices applied to the sample. Select `Adjust` which opens the Sample Slices view where you can define up to 64 slice start points for the currently selected sample.
- **automation:** If On, the table play arrows will advance one row every time the instrument is triggered, and execute only the commands on the new rows. If this is Off, table behavior is normal (play arrows will move at the speed of 1 row per tick)
- **table:** Select a table the instrument will always run. To clone a table here: <span class="minikeys">ALT</span>+(<span class="minikeys">EDIT</span>, <span class="minikeys">ENTER</span>). Make a new table by selecting a higher number not yet in use.

### Auditioning Samples

To preview a sample within the Instrument or Voice screen, press and hold <span class="minikeys">ALT</span>+<span class="minikeys">PLAY</span>. This auditions the currently assigned sample at its configured settings (volume, pan, pitch, etc.) without affecting phrase playback. The <span class="minikeys">PLAY</span> button alone continues to control phrase playback as normal, so you can safely audition samples even while a phrase is playing.

### Sample Slices View

The Sample Slices view provides a visual editor for slice start points. 

With the waveform display selected, you can use <span class="minikeys">LEFT</span> and <span class="minikeys">RIGHT</span> to select a slice point. The `slice` field shows the number of the currently selected slice. Press <span class="minikeys">PLAY</span> to audition only the currently selected slice.

Slices are ordered in strict order and they cannot be reordered. Moving a slice backwards will limit it's movement to the position of the previous slice. Moving a slice forward towards another slice will "shove" the next slice(s) to the new position of the current slice.

Slices are stored per instrument and always reference the currently assigned sample. Changing the instrument's sample when slices are present prompts for confirmation, because accepting the change clears all slice start points.

*Quick edit:*
Focusing the cursor on the sample graph allows to quickly edit the slices

- <span class="minikeys">EDIT</span> + <span class="minikeys">UP</span>/<span class="minikeys">DOWN</span>: Zoom in/out centered on current selected slice (works on any selected field)
- <span class="minikeys">EDIT</span> + <span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span>: Select previous/next slice
- <span class="minikeys">ENTER</span> + <span class="minikeys">UP</span>/<span class="minikeys">DOWN</span>: Move current slice back/forward 1/16th of the screen
- <span class="minikeys">ENTER</span> + <span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span>: Move current slice back/forward 1/64th of the screen (finer adjustment can be done on the position field)

*Auto slicing:* Select the number of slices desired and press the slice button to create evenly distributed slices. Set the number to 1 to quickly delete all slices.

## Sample Import Screen

You can enter the sample import file browser by hitting <span class="minikeys">ENTER</span> <span class="minikeys">ENTER</span> (press the <span class="minikeys">ENTER</span> key twice in quick succession) when you are in the sample field on either the **Sample** or **Original Sample** instrument screen. This double-press behavior provides a quick way to access the sample import file browser from the sample field in the instrument screen.

When in the Project Screen, you can access the sample import file browser with a single press of the <span class="minikeys">ENTER</span> key on the "Import Sample" field.

Samples that you may want to import into a project can be located in any folder but it's recommended to keep them in the folder named `/samples` at the top-level of the sdcard as that is the default location for the sample import file browser to display when you enter it.

When you're done in the Import Screen, you can press <span class="minikeys">NAV</span>+<span class="minikeys">LEFT</span> to return to the screen you came from (either the Instrument Screen or the Project Screen).

Note: sub-directories will be sorted before files, but otherwise the files will be listed in an unspecified order (ie. not necessarily alphabetical order).

![screen capture of sample screen](image/import-screen-small.png)

When entering the import file browser, the current folder is the library root folder `/samples`. All samples (`.wav` files) in that folder are listed.

Use the <span class="minikeys">UP</span> and <span class="minikeys">DOWN</span> arrow keys to navigate through the list of available sample files and subdirectories. Subdirectories are indicated with a `/` prefix. Press <span class="minikeys">EDIT</span> to enter a subdirectory. You can go back to the parent directory by navigating to the `/..` entry and pressing <span class="minikeys">ENTER</span>, or by pressing <span class="minikeys">EDIT</span>+<span class="minikeys">LEFT</span>. Hold down <span class="minikeys">PLAY</span> to audition the currently selected sample wave file. To import the currently selected wave file press <span class="minikeys">ALT</span>+<span class="minikeys">PLAY</span>.

**Single Cycle Waveforms**: Single cycle waveforms are specially marked with a `~` prefix in the file listing. These are WAV files with specific sizes (300 or 1344 bytes) that can be used as oscillators. When imported, they'll automatically be set to oscillator mode in the instrument settings.

A great collection of single cycle waveforms can be found in the [Adventure Kid Sample Library](https://www.adventurekid.se/akrt/waveforms/adventure-kid-waveforms/).

The status bar at the bottom of the screen shows additional information about the selected file and other information such as:
- Current preview volume (vol:XX%)
- File size in bytes
- Available project sample storage size in bytes

### Basic Controls

- Press <span class="minikeys">EDIT</span> to enter a subdirectory
- Navigate to the `/..` entry and press <span class="minikeys">ENTER</span> to go back to the parent directory
- Press <span class="minikeys">EDIT</span>+<span class="minikeys">LEFT</span> to go back to the parent directory
- Hold down <span class="minikeys">PLAY</span> to audition the currently selected sample wave file
- Press <span class="minikeys">ALT</span>+<span class="minikeys">PLAY</span> to import the currently selected wave file
- Use <span class="minikeys">EDIT</span>+<span class="minikeys">UP</span>/<span class="minikeys">DOWN</span> to adjust the preview volume

### Importing Multiple Samples

You can import multiple samples in a single session without leaving the Import View. This powerful workflow allows you to quickly build up your project's sample library:

1. Navigate to a sample file you want to import
2. Press <span class="minikeys">ENTER</span> to import it
3. Navigate to another sample file
4. Press <span class="minikeys">ENTER</span> to import it
5. Repeat as needed for all samples you want to import

Each sample will be added to your project's sample pool. The last imported sample will automatically be assigned to the current instrument. Previously imported samples remain available in your project and can be assigned to other instruments later.

When you're finished importing samples, return to the instrument screen by pressing <span class="minikeys">NAV</span>+<span class="minikeys">LEFT</span>.

{% callout type=note %}
You can also Edit and Rename your sample files as you browse them from the import screen.
{% endcallout %}

### Auditioning Volume Control

The Import View includes a convenient way to adjust the volume when previewing samples:

- **<span class="minikeys">EDIT</span> + <span class="minikeys">UP</span>**: Increase preview volume by 5%
- **<span class="minikeys">EDIT</span> + <span class="minikeys">DOWN</span>**: Decrease preview volume by 5%

The current preview volume is always displayed in the status bar at the bottom of the screen as "vol:XX%" alongside the file size information. 

The preview volume uses a non-linear (quadratic) scale that provides more precise control at lower volumes, making it easier to fine-tune quiet previews. This setting is saved with your project and will be restored when you reload it.

*Note:* While there is a limit of 32 for the number of sub-directory levels, there is a maximum of **256** files per directory. Also please note that while FAT formatted sdcards can support upto *256* characters per filename, picoTracker only supports upto **128** character file names and only with **ASCII** characters. There is also a limit of 256 charactors per full file *path* for any file.


### Supported sample file formats

Sample files must be:
* Uncompressed PCM (Wave/*.wav)
* Unsigned 8 bit; signed 16, 24 or 32 bit; float 32 or 64 bit
* Any samplerate from 8kHz to 192kHz
* Mono or stereo

Bit rate and sample rate are converted on import and saved into the project in the native picoTracker format (16bit/44100Hz/Mono or Stereo)

Sample rate converter selection:
"Import resampler" option in device screen allows to choose the sample rate converter used:
**None:** No sample rate converter is used and only files <=44100Hz can be used, sample rate conversion happens in realtime during playback and uses a linear interpolator
**Linear:** Linear sample rate conversion is used, fast but not band-limited so it may result in aliasing and high frequency loss. Quality is similar to the previous option but higher than 44100Hz files can be imported
**Sinc:** Fastest band-limited sinc interpolation. Higher quality interpolation but still relatively fast. (97dB SNR, 80% bandwidth)
**Sinc Best:** Best band-limited sinc interpolation supported. Quality is comparable to what can be done on a PC at the expense of much slower conversion speed (121db SNR, 90% bandwidth)

Conversion happens on import into the project before loading into the sample pool. Any subsequent project load will not need conversion.

*Conversion speed:* Higher bit rate, higher sample rate and better converters all add up to higher conversion times

## MIDI

## Midi Instrument Screen

![screen capture of MIDI instrument screen](image/midi-screen-small.png)

A MIDI instrument has the following settings:

- **Channel** - This can be set `01`-`16` (in **decimal** not hex!) which is midi channel 1-16 respectively
- **Volume** - The volume any NOTE ON will be sent to your device: FF=127, 00=00
- **Program** - MIDI program change value to send (0x00-0x7F). Program changes for *each* MIDI instrument are sent only once at sequencer start. Setting this to `--` will disable sending program change messages entirely.
- **Length** - Sets note gate length in number of ticks
- **Automation** - When on, the table play arrows will advance one row every time the instrument is triggered, and execute only the commands on the new rows. If this is `Off`, table behavior is normal (play arrows will move at the speed of 1 row per tick)
- **Table**- As above, select a table the instrument will always run. Clone a table here: <span class="minikeys">ALT</span>+<span class="minikeys">EDIT</span>,<span class="minikeys">ENTER</span>. Make a new table by selecting a higher number not yet in use.


### Exporting an Instrument

1. Make sure your instrument has a name set in the "name:" field
   - Each instrument must have a unique name before it can be exported
   - The default instrument type name (e.g., "Sample", "Original Sample", "MIDI", etc.) is not considered a valid name
2. Navigate to the instrument you want to export
3. Select "Export" from the instrument menu
4. Press **OK** to continue after the export is complete message is shown

Exported instruments are stored in `/instruments/` on your SD card. 

**NOTE:** In the future the sample for the instrument will be stored in the same directory as the instrument file but for now is not exported.

### Importing an Instrument

1. Navigate to the instrument you want to import
2. Select "Import" from the instrument menu
3. A file browser will appear showing all available `.pti` files in the `/instruments` directory
4. Select a `.pti` file to import, use the key combo <span class="minikeys">ALT</span>+<span class="minikeys">PLAY</span> to import it
5. The imported instrument will replace the currently selected instrument
6. Press **OK** to continue after the import is complete message is shown



### Tips for Instrument Management

- You can organise your instrument files into subfolders inside the `/instruments` directory but exported files will always be saved in the root `/instruments` directory
- Use descriptive names in the instrument's name field to easily identify them when importing later
- The instrument name is used for the export filename, so ensure it's set before exporting
- Back up your `/instruments` directory when backing up your picoTracker data on your sdcard


## Limitations of instrument performance

The picoTrackers CPU limits the number of simultaneous instruments that can be played at once. The specific limit depends on the instrument type and the settings of each instrument. In general the limit is:
* 4-5 Sample instruments or
* 8 MIDI instruments

Because they are very light weight when it comes to CPU usage, 8 MIDI instruments can be sequenced at once, the limit then coming from the limit of 8 channels (aka tracks) available for sequencing on the picoTracker.

Even given the above limits, it is still possible to mix and match instruments of different types *roughly* within the above limits. For example 2 Sample instruments and 4 MIDI should in theory be possible. Note this only applies to simultaneously sounding (playing) instruments and if care is taken to limit the number of simultaneously sounding instruments, a larger number of instruments can be defined within a project.

## Exporting and Importing Instruments

picoTracker allows you to save and reuse your instrument settings across different projects through the instrument export and import functionality. This feature is particularly useful for building a library of your favorite instruments or using instruments created by other picoTracker users.
