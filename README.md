# picoTracker

picoTracker is a project that aims to provide a low cost open source and DIY hardware music tracker platform. It's firmware started as a modified version of [LittleGPTracker](https://littlegptracker.com/) (a.k.a piggy tracker) but has now diverged in many areas and added alot of new and improved functionality. It implements a user interface similar to the refined track-by-joypad software [*littlesounddj*](http://www.littlesounddj.com/).

## Features

* 8 song channels
* 256 chains
* 128 phrases
* 32 tables
* 16 Sample instruments
* 16 MIDI instruments
* 3 SID instruments
* 3 OPAL instruments
* 8 or 16bit samples up to 44.1kHz, mono or stereo
* 16bit/44.1kHz/Stereo audio output

The picoTracker is powered by an RP2040 microcontroller and supports the following hardware:

* Headphone/Lineout
* TRS MIDI In & Out, USB MIDI Out
* 320x240 2.8in LCD display
* 16MB of Flash
* MicroSD cards upto 32GB for project & sample library storage
* USB-C for MIDI, charging and simple drag&drop firmware upgrades

## Advance

The picoTracker Advance model was launched in November 2025 and has significantly upgraded hardware over the original RP2040 based "pico" model including hi-dpi screen, line-input, internal mic and a speaker. See [here for more information](https://xiphonics.com/products/picotracker).

## Limitations

* The pico will probably struggle with 8 song channels playing at the same time in most cases (the Advance can do 8). There is still room for improvement by either using core0 for partial audio rendering and/or improving the performance of the audio/dsp code (eg. using the hardware interpolator units on the RP2040)
* Cannot load LGPT projects though there is [a basic script to convert projects](util/lgptconvert.py).
* Samples are copied to flash upon load and played from there. Since flash has to be shared with program code, only 8MB is available for it when using the Raspberry Pi Pico, or up to 8MB when using other boards or custom hardware (i.e: the [picoTracker](https://xiphonics.com/products/picotracker-pcb-kit) official hardware kit).
* Instrument count is also pretty low due to memory constraints. 16 Sample, 16 MIDI instruments, 3 SID, 3 OPAL instruments.
* Sample instrument feedback feature has been removed due to memory constraints.
* Soundfont support has been removed to save some memory.


## BUILD

Head over to the [Picotracker Portable Build Guide](docs/BUILD-portable.md)

Head over to the [Build Guide for the original prototype](docs/BUILD.md)

## Remixes

* @ijnekenamay has created a [custom PCB](https://github.com/ijnekenamay/picotracker_alt-pcb/) for the project using mostly off the shelf components

## Experimental Features

The CSID and OPAL synth instruments are expermental and may change in significantly in functionality or even be removed in future releases.


## MANUAL

Head over to the [User Manual](https://manual.xiphonics.com/)

## Remote UI

The [remote UI is available here](https://ui.xiphonics.com/) and its [git repo is here](https://github.com/xiphonics/picotracker_client).

## Get involved!

You can join the fun by following our [contributing guide](docs/CONTRIBUTING.md). 

Join us on [Discord](https://discord.gg/F9nhkd7qj2)

## Development

Head over to the [Developer Guide](docs/DEV.md)

## Want to know more

I posted an article about the development of the picoTracker at [democloid.com](http://democloid.com/2023/04/20/picoTracker.html) and one about the [portable version](https://democloid.com/2023/06/22/picoTrackerPortable.html).
