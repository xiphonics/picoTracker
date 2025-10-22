---
title: Controls & Moves
template: page
---

![labeled photo of Picotracker keymapping](image/pT-buttonMap.png)

The keyboard layout resembles a typical old console game controller (controller button names in brackets):

| `ARROW` keys                               | `NAV` (RT)                              | `ALT` (LT)                              | `ENTER` (A)                               | `EDIT` (B)                               | `PLAY` (Start)                           |
|:-------------------------------------------|:----------------------------------------|:----------------------------------------|:------------------------------------------|:-----------------------------------------|:-----------------------------------------|
| ![arrow keys](image/pt-buttons-arrows.jpg) | ![arrow keys](image/pt-buttons-nav.jpg) | ![arrow keys](image/pt-buttons-alt.jpg) | ![arrow keys](image/pt-buttons-enter.jpg) | ![arrow keys](image/pt-buttons-edit.jpg) | ![arrow keys](image/pt-buttons-play.jpg) |

The `NAV` (`[]`) and `ALT` keys are modifier keys for the `ENTER`, `EDIT` and `ARROW` keys. They are designed to modify
the opposite keys to their location, so `ALT` will modify the `ARROW` keys and `NAV` will modify `ENTER` and `EDIT` (
though there are exceptions).

### Remark about navigation between screens

Screens are getting more detailed from left to right.
Navigating to the next right screen ins only possible if the current cursor position is at field with a filled value.

For example, in the song mode, to switch to the chain view, you need to have a filled value for a chain at the cursor
position.

## Song View

### No Modifier

| Function                              | Key Combination |                   Image                    |
|:--------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                 |  `ARROW` keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback                   |     `PLAY`      |   ![play key](image/pt-buttons-play.jpg)   |
| enter last used value or start with 0 |     `ENTER`     |  ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function                                               | Key Combination |                       Image                        |
|:-------------------------------------------------------|:---------------:|:--------------------------------------------------:|
| Switch to Project view                                 |   `NAV + UP`    |    ![nav + up key](image/pt-buttons-nav_up.jpg)    |
| Switch to Mixer view                                   |  `NAV + DOWN`   |  ![nav + down key](image/pt-buttons-nav_down.jpg)  |
| Switch to Chain view (if current position has a chain) |  `NAV + RIGHT`  | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| Stop playback of selected chain, once it finished      |  `NAV + PLAY`   |  ![nav + play key](image/pt-buttons-nav_play.jpg)  |
| solo selected track                                    |  `NAV + ENTER`  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) |
| mute selected track                                    |  `NAV + EDIT`   | ![nav + edit keys](image/pt-buttons-nav_edit.jpg)  |
| reset all muted & soloed tracks                        |   `NAV + ALT`   |  ![nav + edit keys](image/pt-buttons-nav_alt.jpg)  |

#### Solo & Mute

When `NAV` is released first, the (solo / mute) mode will be kept active (toggled).
Pressing the same key combination but releasing `NAV` first will reset the (solo / mute) mode of the currently selected
track.

To reset all muted and soloed tracks, press `NAV + ALT`.

### ALT Modifier

| Function                                                                                              |      Key Combination       |                              Image                              |
|:------------------------------------------------------------------------------------------------------|:--------------------------:|:---------------------------------------------------------------:|
| Jump up/down to next populated row after a blank row                                                  |    `ALT + (UP / DOWN)`     |     ![alt + up/down keys](image/pt-buttons-alt_up_down.jpg)     |
| Nudge tempo down/up                                                                                   |   `ALT + (LEFT / RIGHT)`   | ![nav + alt + left/right](image/pt-buttons-alt_left_right.jpg)  |
| query current row for playback                                                                        |        `ALT + PLAY`        |       ![nav + alt + play](image/pt-buttons-alt_play.jpg)        |
| reset all muted & soloed tracks                                                                       |        `ALT + NAV`         |       ![nav + alt + enter](image/pt-buttons-nav_alt.jpg)        |
| cut the current cursor position if filled, paste otherwise                                            |       `ALT + ENTER`        |          ![arrow keys](image/pt-buttons-alt_enter.jpg)          |
| Clone: Overwrite current highlighted Item with a copy of itself using the next unused Item available. |    `ALT + EDIT + ENTER`    | ![alt + edit + enter keys](image/pt-buttons-alt_edit_enter.jpg) |
| start selection                                                                                       |       `ALT + EDIT `        |        ![edit + alt keys](image/pt-buttons-alt_edit.jpg)        |
| start selection with row selected                                                                     |    `ALT + EDIT + EDIT`     |       ![edit + alt keys](image/pt-buttons-alt_edit2.jpg)        |
| start selection with current screen selected                                                          | `ALT + EDIT + EDIT + EDIT` |       ![edit + alt keys](image/pt-buttons-alt_edit3.jpg)        |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                             | Key Combination |                     Image                     |
|-----------------------------------------|:---------------:|:---------------------------------------------:|
| change selection                        |    `ARROWS`     |  ![arrow keys](image/pt-buttons-arrows.jpg)   |
| increase selection to full row / screen |  `ALT + EDIT`   | ![nav + enter](image/pt-buttons-alt_edit.jpg) |
| copy selection to clipboard             |     `EDIT`      |   ![arrow keys](image/pt-buttons-edit.jpg)    |
| cut selection to clipboard              |  `ALT + ENTER`  | ![arrow keys](image/pt-buttons-alt_enter.jpg) |

### EDIT Modifier

| Function                                                                     |     Key Combination     |                             Image                              |
|:-----------------------------------------------------------------------------|:-----------------------:|:--------------------------------------------------------------:|
| Cuts the current Highlighted Item.                                           |     `EDIT + ENTER`      |     ![edit + enter keys](image/pt-buttons-edit_enter.jpg)      |
| start selection                                                              |      `EDIT + ALT`       |       ![edit + alt keys](image/pt-buttons-alt_edit.jpg)        |
| Start immediate playback                                                     |      `EDIT + PLAY`      |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       |
| Page up/down in Song Screen.                                                 |  `EDIT + (UP / DOWN)`   |    ![edit + up/down keys](image/pt-buttons-edit_updown.jpg)    |
| Switch between Song and Live Mode                                            | `EDIT + (LEFT / RIGHT)` | ![edit + left/right keys](image/pt-buttons-edit_leftright.jpg) |
| mute selected track                                                          |      `EDIT + NAV`       |       ![nav + edit keys](image/pt-buttons-nav_edit.jpg)        |
| \[advance only\] Sample recording, only accessible when sequencer is stopped |     `EDIT` + `PLAY`     |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       |

### ENTER Key Combinations

| Function                                                   |     Key Combination      |                               Image                               |
|:-----------------------------------------------------------|:------------------------:|:-----------------------------------------------------------------:|
| insert if cursor position is empty                         |         `ENTER`          |             ![enter key](image/pt-buttons-enter.jpg)              |
| Insert next unused chain                                   |      `ENTER, ENTER`      |          ![enter key twice](image/pt-buttons-enter2.jpg)          |
| cut the current cursor position if filled, paste otherwise |      `ENTER + ALT`       |           ![arrow keys](image/pt-buttons-alt_enter.jpg)           |
| Change chain at cursor position by 0x10                    |  `ENTER + (UP / DOWN)`   |     ![enter + arrow keys](image/pt-buttons-enter_up_down.jpg)     |
| Change chain at cursor position by 0x01                    | `ENTER + (RIGHT / LEFT)` | ![enter + right/left keys](image/pt-buttons-enter_left_right.jpg) |
| solo selected track                                        |      `ENTER + NAV`       |        ![nav + enter key](image/pt-buttons-nav_enter.jpg)         |

## Chain View

### No Modifier

| Function                               | Key Combination |                   Image                    |
|:---------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                  |  `ARROW` keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback of current chain   |     `PLAY`      |   ![play key](image/pt-buttons-play.jpg)   |
| enter last used phrase or start with 0 |     `ENTER`     |  ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function              | Key Combination |                       Image                        |
|:----------------------|:---------------:|:--------------------------------------------------:|
| switch to song view   |  `NAV + LEFT`   |  ![nav + left key](image/pt-buttons-nav_left.jpg)  |
| switch to phrase view |  `NAV + RIGHT`  | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| play chain in song    |  `NAV + PLAY`   | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| unmute all            |   `NAV + ALT`   |   ![nav + alt key](image/pt-buttons-nav_alt.jpg)   |
| toggle mute           |  `NAV + EDIT`   |  ![nav + edit key](image/pt-buttons-nav_edit.jpg)  |
| switch solo mode      |  `NAV + ENTER`  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) |

### ALT Modifier

| Function               |    Key Combination    |                             Image                              |
|:-----------------------|:---------------------:|:--------------------------------------------------------------:|
| unmute all             |      `ALT + NAV`      |         ![alt + nav key](image/pt-buttons-nav_alt.jpg)         |
| clone current position | ` ALT + EDIT + ENTER` | ![alt + edit + enter key](image/pt-buttons-alt_edit_enter.jpg) |
| paste clipboard        |     `ALT + ENTER`     |       ![alt + enter key](image/pt-buttons-alt_enter.jpg)       |
| start selection        |     `ALT + EDIT`      |        ![alt + edit key](image/pt-buttons-alt_edit.jpg)        |

### EDIT Modifier

| Function                                                                     |      Key Combination       |                                Image                                 |
|:-----------------------------------------------------------------------------|:--------------------------:|:--------------------------------------------------------------------:|
| warp to (previous / next) channel                                            |  `EDIT + (LEFT / RIGHT)`   | ![edit + horizontal arrow keys](image/pt-buttons-edit_leftright.jpg) |
| warp to (previous / next) chain of current channel                           |    `EDIT + (UP / DOWN)`    |   ![edit + vertical arrow keys](image/pt-buttons-edit_updown.jpg)    |
| cut current position into clipboard                                          |       `EDIT + ENTER`       |         ![edit + enter key](image/pt-buttons-edit_enter.jpg)         |
| clone current position                                                       |    `EDIT + ALT + ENTER`    |    ![edit + alt + enter key](image/pt-buttons-alt_edit_enter.jpg)    |
| toggle mute                                                                  |        `EDIT + NAV`        |           ![edit + nav key](image/pt-buttons-nav_edit.jpg)           |
| start selection                                                              |        `ALT + EDIT`        |           ![alt + edit key](image/pt-buttons-alt_edit.jpg)           |
| start selection with row selected                                            |    `ALT + EDIT + EDIT`     |       ![alt + edit twice keys](image/pt-buttons-alt_edit2.jpg)       |
| start selection with current screen selected                                 | `ALT + EDIT + EDIT + EDIT` |      ![alt + edit thrice keys](image/pt-buttons-alt_edit3.jpg)       |
| \[advance only\] Sample recording, only accessible when sequencer is stopped |      `EDIT` + `PLAY`       |         ![edit + play keys](image/pt-buttons-edit_play.jpg)          |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                         |     Key Combination      |                                    Image                                    |
|-------------------------------------|:------------------------:|:---------------------------------------------------------------------------:|
| change selection                    |         `ARROWS`         |                 ![arrow keys](image/pt-buttons-arrows.jpg)                  |
| copy selection to clipboard         |          `EDIT`          |                   ![edit key](image/pt-buttons-edit.jpg)                    |
| toggle mute                         |       `EDIT + NAV`       |              ![edit + nav key](image/pt-buttons-nav_edit.jpg)               |
| update selection values by +/- 0x10 |  `ENTER + (UP + DOWN)`   |     ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)      |
| update selection values by +/- 0x01 | `ENTER + (LEFT + RIGHT)` | ![enter key + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| cut the current selection           |      `ALT + ENTER`       |             ![alt + enter key](image/pt-buttons-alt_enter.jpg)              |
| switch solo mode                    |      `NAV + ENTER`       |             ![nav + enter key](image/pt-buttons-nav_enter.jpg)              |
| switch to song view                 |       `NAV + LEFT`       |              ![nav + left key](image/pt-buttons-nav_left.jpg)               |
| switch to phrase view               |      `NAV + RIGHT`       |             ![nav + right key](image/pt-buttons-nav_right.jpg)              |
| play chain                          |          `PLAY`          |                   ![play key](image/pt-buttons-play.jpg)                    |
| play chain in song                  |       `NAV + PLAY`       |              ![nav + play key](image/pt-buttons-nav_play.jpg)               |
| unmute all                          |       `ALT + NAV`        |               ![alt + nav key](image/pt-buttons-nav_alt.jpg)                |

### ENTER Key Combinations

| Function                        |     Key Combination      |                                  Image                                  |
|:--------------------------------|:------------------------:|:-----------------------------------------------------------------------:|
| update cursor value by +/- 0x10 |  `ENTER + (UP / DOWN)`   |   ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| update cursor value by +/- 0x01 | `ENTER + (LEFT / RIGHT)` | ![enter + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| paste clipboard                 |      `ENTER + ALT`       |           ![enter + alt key](image/pt-buttons-alt_enter.jpg)            |
| switch solo mode                |      `ENTER + NAV`       |           ![enter + nav key](image/pt-buttons-nav_enter.jpg)            |

## Phrase View

### No Modifier

| Function                                                                                   | Key Combination |                   Image                    |
|:-------------------------------------------------------------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                                                                      |  `ARROW` keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback of current phrase                                                      |     `PLAY`      |   ![play key](image/pt-buttons-play.jpg)   |
| enter last used value or start with 0                                                      |     `ENTER`     |  ![enter key](image/pt-buttons-enter.jpg)  |
| in instrument column: insert first unused instrument number                                | `ENTER`,`ENTER` |  ![enter key](image/pt-buttons-enter.jpg)  |
| in argument column of TBL command: insert first unused table number (TODO: see issue #753) | `ENTER`,`ENTER` |  ![enter key](image/pt-buttons-enter.jpg)  |
| in note / instrument column: audition note                                                 |  `ENTER` held   |  ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function                  | Key Combination |                       Image                        |
|:--------------------------|:---------------:|:--------------------------------------------------:|
| switch to chain view      |  `NAV + LEFT`   |  ![nav + left key](image/pt-buttons-nav_left.jpg)  |
| switch to instrument view |  `NAV + RIGHT`  | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| switch to table view      |  `NAV + DOWN`   |  ![nav + down key](image/pt-buttons-nav_down.jpg)  |
| switch to groove view     |   `NAV + UP`    |    ![nav + up key](image/pt-buttons-nav_up.jpg)    |
| play phrase in song       |  `NAV + PLAY`   |  ![nav + play key](image/pt-buttons-nav_play.jpg)  |
| unmute all                |   `NAV + ALT`   |   ![nav + alt key](image/pt-buttons-nav_alt.jpg)   |
| switch mute mode          |  `NAV + EDIT`   |  ![nav + edit key](image/pt-buttons-nav_edit.jpg)  |
| switch solo mode          |  `NAV + ENTER`  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) |

### ALT Modifier

| Function                                                                        |   Key Combination    |                             Image                              |
|:--------------------------------------------------------------------------------|:--------------------:|:--------------------------------------------------------------:|
| unmute all                                                                      |     `ALT + NAV`      |         ![alt + nav key](image/pt-buttons-nav_alt.jpg)         |
| paste clipboard                                                                 |    `ALT + ENTER`     |       ![alt + enter key](image/pt-buttons-alt_enter.jpg)       |
| start selection                                                                 |     `ALT + EDIT`     |        ![alt + edit key](image/pt-buttons-alt_edit.jpg)        |
| clone current instrument (cursor in note / instrument column)                   | `ALT + EDIT + ENTER` | ![alt + edit + enter key](image/pt-buttons-alt_edit_enter.jpg) |
| clone current table (cursor in note / instrument column) (TODO: see issue #753) | `ALT + EDIT + ENTER` | ![alt + edit + enter key](image/pt-buttons-alt_edit_enter.jpg) |

### EDIT Modifier

| Function                                                                     |      Key Combination       |                                Image                                 |
|:-----------------------------------------------------------------------------|:--------------------------:|:--------------------------------------------------------------------:|
| warp to (previous / next) track                                              |  `EDIT + (LEFT / RIGHT)`   | ![edit + horizontal arrow keys](image/pt-buttons-edit_leftright.jpg) |
| warp to (previous / next) phrase in chain                                    |    `EDIT + (UP / DOWN)`    |   ![edit + vertical arrow keys](image/pt-buttons-edit_updown.jpg)    |
| cut current position into clipboard                                          |       `EDIT + ENTER`       |         ![edit + enter key](image/pt-buttons-edit_enter.jpg)         |
| toggle mute                                                                  |        `EDIT + NAV`        |           ![edit + nav key](image/pt-buttons-nav_edit.jpg)           |
| start selection                                                              |        `EDIT + ALT`        |           ![alt + edit key](image/pt-buttons-alt_edit.jpg)           |
| start selection with row selected                                            |    `ALT + EDIT + EDIT`     |       ![alt + edit twice keys](image/pt-buttons-alt_edit2.jpg)       |
| start selection with current screen selected                                 | `ALT + EDIT + EDIT + EDIT` |      ![alt + edit thrice keys](image/pt-buttons-alt_edit3.jpg)       |
| \[advance only\] Sample recording, only accessible when sequencer is stopped |      `EDIT` + `PLAY`       |         ![edit + play keys](image/pt-buttons-edit_play.jpg)          |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                             |     Key Combination      |                                  Image                                   |
|-----------------------------------------|:------------------------:|:------------------------------------------------------------------------:|
| change selection                        |         `ARROWS`         |                ![arrow keys](image/pt-buttons-arrows.jpg)                |
| increase selection to full row / screen |       `ALT + EDIT`       |              ![nav + enter](image/pt-buttons-alt_edit.jpg)               |
| copy selection to clipboard             |          `EDIT`          |                  ![edit key](image/pt-buttons-edit.jpg)                  |
| cut the current selection               |      `ALT + ENTER`       |            ![alt + enter key](image/pt-buttons-alt_enter.jpg)            |
| update selection values by +/- 0x10     |  `ENTER + (UP + DOWN)`   |    ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| update selection values by +/- 0x01     | `ENTER + (LEFT + RIGHT)` | ![enter  + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |

### ENTER Key Combinations

| Function                        |     Key Combination      |                                  Image                                  |
|:--------------------------------|:------------------------:|:-----------------------------------------------------------------------:|
| update cursor value by +/- 0x10 |  `ENTER + (UP / DOWN)`   |   ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| update cursor value by +/- 0x01 | `ENTER + (LEFT / RIGHT)` | ![enter + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| paste clipboard                 |      `ENTER + ALT`       |           ![enter + alt key](image/pt-buttons-alt_enter.jpg)            |
| switch solo mode                |      `ENTER + NAV`       |           ![enter + nav key](image/pt-buttons-nav_enter.jpg)            |

[//]: # (TODO:)

[//]: # (instrument)

[//]: # (table)

[//]: # (project)
