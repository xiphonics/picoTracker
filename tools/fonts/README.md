# What is this?

Scripts and raw data to generate the font imports for picoTracker/Advance and picotracker_client

## `font.svg` and `font_adv.svg`

Vector graphics containing all special characters for both fonts. font_adv.svg contains all letters/numbers/.. in Courier Code Pro and Ubuntu Mono. font.svg contains the wide font letters/numbers/...

## `font.png` and `font_adv.png`

Exported png files with transparent background from the two svgs mentioned above.

## `font_hourglass.png` and `font_yousquared.png`

Generated png files for the Hourglass and YouSquared fonts from the code in the repo. Makes editing and fixing easier. Those files are also used for the picotracker_client.

## Converter python scripts

1) `import-adv.py`

    Converts `font_adv.png` into a single C header containing all 224 characters.

2) `import.py`

    Converts font png files to C headers. Takes a few arguments to allow exporting character ranges, import from different files etc. Run without parameters for help.

    Most used:

    a) Export special characters only:

    `python3 import.py font_hourglass.png --start 128 --end 255 --name SPECIAL_CHARS`

    b) Export wide font regular range

    `python3 import.py font_wide.png --start 32 --end 127 --name FONT_WIDE`