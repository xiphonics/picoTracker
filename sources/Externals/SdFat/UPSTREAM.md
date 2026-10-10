# SdFat integration

Library sources in `src/` are from upstream SdFat tag **2.3.1**:
https://github.com/greiman/SdFat/tree/2.3.1

The CMake files are picoTracker integration. Configuration is supplied by
`Adapters/picoTracker/sdcard/sdfatConfig.h`. The sole library-source integration
change selects our existing RP2040 `SdioCard` declaration in `SdCard/SdCard.h`;
the declaration and implementation live under `Adapters/picoTracker/sdcard/`.
The driver's new `readSDS()` interface returns failure, as its other unsupported
card-register operations do. Normal filesystem I/O does not use it.

There are no patches for working directories or physical dot entries.
Examples and generated HTML documentation have not been regenerated.

Upstream's FAT32 runtime does not update the advisory FSInfo free-cluster
count (also true of our previous 2.2.0 sources). A desktop filesystem check
may report a stale count after writes to a desktop-formatted card; the FAT
allocation table and directory contents are maintained independently.
