# A ZX-Origins Typeface

Thank you for downloading one of the fonts in my ZX Origins collection. You will see a number of files in this download for the various formats this font is supplied in. They are as follows:

## Formats

### Sinclair ZX Spectrum

Inside the **Spectrum** folder you will find each font has a `.ch8` file that is a RAW 768-byte file that can be loaded directly into BASIN, AGD or other Spectrum environments. Alternatively you can load it into any available RAM address and then `POKE 23606` and `23607` with the appropriate RAM address minus 256 bytes.

Each font also has a `.fzx` file containing a proportional conversion of the font for use with the [FZX Proportional Font Renderer](https://github.com/z88dk/z88dk/tree/master/libsrc/_DEVELOPMENT/font/fzx) available for the ZX Spectrum. Z80 assembler-compatible files containing `defb` statements can be found in the `source` folder with the suffix `.z80.asm`.

A `.tap` tape image is provided which can be either written to a physical tape, loaded in via a DivMMC or mounted in an emulator. It contains all the fonts for the given typeface complete with a built-in demonstration program that will allow all the fonts to be previewed in any color combination.

### Acorn BBC Micro/Electron

Inside the **BBC** folder you will find each font has a `.bbc` file that can be copied to your own discs and `*EXEC`'ed once you have set `*FX 20,7` and carved out space for them with `PAGE=&1F00`. It is recommended you `*EXEC` them from your `!BOOT` file. If you need to execute them from your own BASIC then instead you should use the [BFont](http://mdfs.net/Apps/Font/) load routine by J.G.Harston in 0/VDU mode.

Additionally a DFS-formatted `.ssd` disk image is provided for use with emulators contains a demonstration program showing the fonts available for this typeface in any color combination in low, medium or high-resolution modes.

### Commodore 64

The **C64** folder contains two `.64c` files (raw in PETSCI order with a 2 byte extra header) - one with upper-case only and more line drawing characters (`.upper`) - and the other containing both upper and lower case (`.both`).

Additionally there is a combined `.bin` file containing both the fonts in a raw format that can be used as a character ROM in an emulator or otherwise imported into your dev system.

### Amstrad CPC

The **CPC** folder contains a `.bas` file representing ASCII BASIC versions of the font definition for Amstrad BASIC starting at line 9000. You can use a tool such as WinAPE's "Auto Type" feature to paste this into your BASIC program. When run this code will redefine the current character set to match. Note that ZX Origins is a subset of the Amstrad CPC character set and so will not include various symbols and non-Latin characters.

Alternatively a `.dsk` file is also included which contains a CPC 664/6128 DATA disk image that is ready to use. Either run them directly or `MERGE` them with your BASIC program. Ensure lines 9000-9980 are free in your program as they will be overwritten by the MERGE.

### Game Boy

The **GameBoy** folder contains multiple `.png` files that can be loaded into a Game Boy Studio for developing games for the Nintendo Game Boy. Each font has a regular (light), a `-dark` (inverse), and a `-var` proportional/variable-width versions. Right now only the standard Spectrum character set is represented so ASCII plus pound and copyright symbols.

### Atari 8-bit

The **Atari8** folder contains a `.fnt` file for each font which is the RAW font data with ATASCII ordering with line-drawing characters from the original machine set.

### CoCo (Tandy Color Computer/TRS-80)

Files are included ready for use with the [CoCoVGA board](http://www.cocovga.com/) which allows redefined characters. Each `.CHR` file contains a single font in RS-DOS binary format complete with standard 5-byte header and trailer configured to load at `$E00`.

A `.dsk` disk image is also included containing copies of each of the fonts in the family within a 35-track Disk Basic image file produced by [CMOC](http://perso.b2b2c.ca/~sarrazip/dev/cmoc-manual.html).

### PC

### BDF (Glyph Bitmap Distribution Format)

Standard bitmap font format used by many Windows and Mac tools. If you need to extend the character set out I would start here as it allows for large character sets etc and is still purely bitmap.

### PSF (PC Screen Font)

Font file capable of being used by Linux as console fonts.

### TTF (TrueType font)

A Windows, Mac and Linux friendly scalable version of the font. It is very hard to preserve the original bitmap sharply across platforms in a TrueType container but I've done what I can. This looks best at multiples of 8px on Windows. Experiment with other sizes or metrics as you need depending on your OS.

### WOFF (Web font)

A web font for older browsers and the Godot game engine.

### WOFF2 (Web font)

A web font for modern browsers containing a fully Brotli compressed web-friendly version of the TrueType font for use within web pages.

## Other systems

In the `Source` folder you will find:

- C-style headers containing `static const` arrays with the suffix `.h`
- Motorola 68000 assembler files containing `DC.B` statements with the suffix `.68000.asm`
- Intel x86 assembler files containing `db` statements with the suffix `.x86.asm`
- Zilog Z80 assembler files containing `defb` statements with the suffix `.z80.asm`
- MOS 6502 assembler files containing `.byte` statements with the suffix `.6502.asm`

Please note that all these files contain the raw glyphs in ASCII order with Sinclair-specific modifications:

- ↑ (up-arrow) at 0x5E instead of ^ (caret) except where I've drawn them as a caret
- £ (UK pound sign) at 0x60 instead of ` (grave accent)
- © (copyright sign) at 0x7F instead of the DEL control code

## Further conversion

If you wish to convert this font to other formats the BDF file is a good starting point.

Alternatively check out John Elliot's PSFTOOLS which can turn the .psf file included into BBC, Wyse, C code, FNT and others formats. I would recommend however sticking with the TTF and WOFF2 for scalable versions as they have been carefully optimized to reduce smoothing at specific sizes.

Many of the conversions are achieved using my open-source [PixelWorld tool](https://github.com/damieng/pixelworld).

I may include additional formats in future updates of ZX Origins if there is demand.

## Thanks

My sincere thanks to:

- [John Elliot](https://www.seasip.info/) for his amazing [psftools](https://www.seasip.info/Unix/PSF/) which helped streamline the process.
- [Paul van der Laan](http://type-invaders.com) for lending his expertise and research into how to make 8x8 bitmap fonts perfect in [FontLab Studio 5](https://www.fontlab.com/font-editor/fontlab-studio-5/).
- [Paul Dunn](https://github.com/ZXDunny) for the BASIN editor that is my 8x8 bitmap font designer of choice
- [Arda Erdikmen](https://github.com/ref-xx) for improvements to [BASIN](https://github.com/ref-xx/basinc)
- [Santiago Crespo](https://gameboys.es/) for all his help in getting the GameBoy Studio versions right
- [J.G.Harston](https://mdfs.net/) for the BBC font-loading routines [BFont](https://mdfs.net/Apps/Font/) and the [mkimg tool](https://mdfs.net/Apps/DiskTools/) used to automate .ssd creation
- The team behind [iDSK](https://github.com/cpcsdk/idsk) used to prepare the Amstrad CPC disk images
- To Brendan Donahe for the [CoCoVGA](http://cocovga.com/about/) scripts and conversion tool

## Licence

This font is part of the ZX Origins font collection Copyright (c) 1988-2023 Damien Guard.

Formal licenses are complicated and burdensome so here's the deal. These are some acceptable use examples:

1. Use it in your game (commercial or non-commercial)
2. Print something on a t-shirt
3. Use the embedded font file on your site
4. Set your favorite terminal or OS to use it

Ideally with a credit like "<fontname> font by DamienG https://damieng.com/zx-origins" _if_ you have a credits section. If you don't that's fine. Either way dropping me an email at damieng@gmail.com to let me know what you used it for **is appreciated**.

The only prohibited use is redistributing this font as a font. i.e. re-hosting the files on your own site or bundling it with other art assets or using it to create assets. I have put a lot of time into these and my only reward is seeing download counts on my site which is ad-free so it seems unfair other people would re-host these files on their site and make $ from Google AdWords etc.

If you need to modify the font for your usage - either to add characters (this collection is pure ASCII + copyright + UK pound sign right now) or if a few are bothering you just change the credit to "Font based on <fontname> by DamienG" or something.

Thanks and enjoy!

[)amien
https://damieng.com/zx-origins/
