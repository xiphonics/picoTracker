# picoTracker

picoTracker is a low cost DIY hardware device based on a modified version of [LittleGPTracker](https://www.littlegptracker.com/) running on the Raspberry Pi Pico and some additional hardware.

## Features

* 8 song channels
* 128 chains
* 128 phrases
* 32 tables
* 16 Sample instruments
* 16 MIDI instruments
* 1MB sample memory

## Limitations
* Will probably struggle with 8 song channels playing at the same time in most cases. I modified the source to parametrically reduce the total songs, but didn't want to make the decision of supporting only 6 songs or so just yet. There is still room for improvement by either multithreading or increasing CPU frequency.
* Cannot load LGPT projects (thou it shouldn't be too hard to do, for projects that fit the specs above).
* Samples are played copied to flash upon load and played from there. Since flash has to be shared with program code, only 1MB is available for it.
* Instrument count is also pretty low due to memory constraints.
* Sample instrument feedback feature has been removed due to memory constraints.
* Sample fonts support has been removed to save some memory (thou it could be added back).

## Known issues
* Loading samples into project will leak memory
  * Workaround: load all necessary samples, save, restart machine and reload project
* Loading a project will also leak memory. Not a problem in normal operation, but it means we cannot currently quit a project and load a new one
  * Workaround: Have you tried turning it off and on again?

## TODO/Improvements
* Fix memory leak issues and modernize code
* Add new instruments (SID WIP)
* Create custom PCB
* Improve SDIO performance and explore further playing samples from SD
* Further memory savings and bring back some features/instruments

## BUILD
Head over to the [Build Guide](docs/BUILD.md)

## Development
Head over to the [Developer Guide](docs/DEV.md)

## Want to know more
I posted some articles about the development of the picoTracker at [democloid.com](http://democloid.com)
