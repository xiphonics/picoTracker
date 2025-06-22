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

## Song View

### No Modifier

| Function              | Key Combination |                   Image                    |
|:----------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen |  `ARROW` keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback   |     `PLAY`      |   ![play key](image/pt-buttons-play.jpg)   |

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

When `NAV` is released first, the (solo / mute) mode will be kept active until reset with `NAV + ALT`.

### ALT Modifier

| Function                                                                                              | Key Combination            |                              Image                               |
|:------------------------------------------------------------------------------------------------------|:---------------------------|:----------------------------------------------------------------:|
| Jump up/down to next populated row after a blank row                                                  | `ALT + (UP / DOWN)`        |     ![alt + up/down keys](image/pt-buttons-alt_up_down.jpg)      |
| Nudge tempo down/up                                                                                   | `ALT + (LEFT / RIGHT)`     |  ![nav + alt + left/right](image/pt-buttons-alt_left_right.jpg)  |
| query current row for playbck                                                                         | `ALT + PLAY`               |        ![nav + alt + play](image/pt-buttons-alt_play.jpg)        |
| reset all muted & soloed teacks                                                                       | `ALT + NAV`                |        ![nav + alt + enter](image/pt-buttons-nav_alt.jpg)        |
| cut the current cursor position if filled, paste otherwise                                            | `ALT + ENTER`              |          ![arrow keys](image/pt-buttons-alt_enter.jpg)           |
| Clone: Overwrite current highlighted Item with a copy of itself using the next unused Item available. | `ALT + (EDIT + ENTER)`     | ![alt + (adit, enter) keys](image/pt-buttons-alt_edit_enter.jpg) |
| start selection                                                                                       | `ALT + EDIT `              |        ![edit + alt keys](image/pt-buttons-alt_edit.jpg)         |
| start selection with row selected                                                                     | `ALT + EDIT + EDIT`        |        ![edit + alt keys](image/pt-buttons-alt_edit2.jpg)        |
| start selection with current screen selected                                                          | `ALT + EDIT + EDIT + EDIT` |        ![edit + alt keys](image/pt-buttons-alt_edit3.jpg)        |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                                                | Key Names     |                     Image                      |
|------------------------------------------------------------|---------------|:----------------------------------------------:|
| change selection                                           | `ARROWS`      |   ![arrow keys](image/pt-buttons-arrows.jpg)   |
| copy selection to clipboard                                | `EDIT`        |    ![arrow keys](image/pt-buttons-edit.jpg)    |
| queue selection for playback                               | `PLAY`        |    ![arrow keys](image/pt-buttons-play.jpg)    |
| stop                                                       | `NAV + PLAY`  |  ![arrow keys](image/pt-buttons-nav_play.jpg)  |
| cut the current cursor position if filled, paste otherwise | `ALT + ENTER` | ![arrow keys](image/pt-buttons-alt_enter.jpg)  |
| switch solo mode                                           | `NAV + ENTER` | ![nav + enter](image/pt-buttons-nav_enter.jpg) |
| unmute all                                                 | `ALT + NAV`   |  ![nav + enter](image/pt-buttons-nav_alt.jpg)  |

### EDIT Modifier

| Function                                                                     | Key Combination         |                             Image                              |
|:-----------------------------------------------------------------------------|:------------------------|:--------------------------------------------------------------:|
| Cuts the current Highlighted Item.                                           | `EDIT + ENTER`          |     ![edit + enter keys](image/pt-buttons-edit_enter.jpg)      |
| start selection                                                              | `ALT + EDIT`            |       ![edit + alt keys](image/pt-buttons-alt_edit.jpg)        |
| Start immediate playback                                                     | `EDIT + PLAY`           |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       |
| Page up/down in Song Screen.                                                 | `EDIT + (UP / DOWN)`    |    ![edit + up/down keys](image/pt-buttons-edit_updown.jpg)    |
| Switch between Song and Live Mode                                            | `EDIT + (LEFT / RIGHT)` | ![edit + left/right keys](image/pt-buttons-edit_leftright.jpg) |
| \[advance only\] Sample recording, only accessible when sequencer is stopped | `EDIT` + `PLAY`         |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       |

### ENTER Key Combinations

| Function                                                   | Key Combination          |                               Image                               |
|:-----------------------------------------------------------|:-------------------------|:-----------------------------------------------------------------:|
| insert if cursor position is empty                         | `ENTER`                  | ![enter key](image/pt-buttons-enter.jpg)                          |
| Insert next unused chain                                   | `ENTER, ENTER`           | ![enter key twice](image/pt-buttons-enter2.jpg)                   |
| Change chain at cursor position by 0x10                    | `ENTER + (UP / DOWN)`    | ![enter + arrow keys](image/pt-buttons-enter_up_down.jpg)         |
| Change chain at cursor position by 0x01                    | `ENTER + (RIGHT / LEFT)` | ![enter + right/left keys](image/pt-buttons-enter_left_right.jpg) |
| cut the current cursor position if filled, paste otherwise | `ALT + ENTER`            | ![enter + alt keys](image/pt-buttons-alt_enter.jpg)               |

## Chain View

### No Modifier

| function                                  | key combination | image                                               |
|:------------------------------------------|:----------------|:------------------------------------------:|
| Move cursor on screen                     | `ARROW` keys    | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback of current chain      | `PLAY`          | ![play key](image/pt-buttons-play.jpg)     |
| enter last used phrase or start with `00` | `ENTER`         | ![enter key](image/pt-buttons-enter.jpg)   |

### NAV Modifier

| function              | key combination | image                                              |
|:----------------------|:----------------|:--------------------------------------------:|
| switch to song view   | `NAV + LEFT`    | ![enter key](image/pt-buttons-nav_left.jpg)  |
| switch to phrase view | `NAV + RIGHT`   | ![enter key](image/pt-buttons-nav_right.jpg) |
| play chain in song    | `NAV + PLAY`    | ![enter key](image/pt-buttons-nav_play.jpg)  |
| unmute all            | `NAV + ALT`     | ![enter key](image/pt-buttons-nav_alt.jpg)   |
| toggle mute           | `EDIT + NAV`    | ![enter key](image/pt-buttons-nav_edit.jpg)  |
| switch solo mode      | `ENTER + NAV`   | ![enter key](image/pt-buttons-nav_enter.jpg) |

### ALT Modifier

| function               | key combination      | image                                             |
|:-----------------------|:---------------------|:-------------------------------------------------:|
| unmute all             | `NAV + ALT`          | ![enter key](image/pt-buttons-nav_alt.jpg)        |
| clone current position | `EDIT + ALT + ENTER` | ![enter key](image/pt-buttons-alt_edit_enter.jpg) |
| paste clipboard        | `ENTER + ALT`        | ![enter key](image/pt-buttons-alt_enter.jpg)      |

### EDIT Modifier

| function                                           | key combination         | image                                              |
|:---------------------------------------------------|:------------------------|:--------------------------------------------------:|
| warp to (previous / next) channel                  | `EDIT + (LEFT / RIGHT)` | ![enter key](image/pt-buttons-edit_leftright.jpg) |
| warp to (previous / next) chain of current channel | `EDIT + (UP / DOWN)`    | ![enter key](image/pt-buttons-edit_updown.jpg)    |
| cut current position into clipboard                | `EDIT + ENTER`          | ![enter key](image/pt-buttons-edit_enter.jpg)     | 
| clone current position                             | `EDIT + ALT + ENTER`    | ![enter key](image/pt-buttons-alt_edit_enter.jpg) |
| toggle mute                                        | `EDIT + NAV`            | ![enter key](image/pt-buttons-nav_edit.jpg)       |

### ENTER Key Combinations

| function                                                 | key combination          | image                                               |
|:---------------------------------------------------------|:-------------------------|:---------------------------------------------------:|
| update cursor value by +/- 0x10                          | `ENTER + (UP / DOWN)`    | ![enter key](image/pt-buttons-enter_up_down.jpg)    |
| update cursor value by +/- 0x01                          | `ENTER + (LEFT / RIGHT)` | ![enter key](image/pt-buttons-enter_left_right.jpg) |
| paste clipboard                                          | `ENTER + ALT`            | ![enter key](image/pt-buttons-alt_enter.jpg)        |
| switch solo mode                                         | `ENTER + NAV`            | ![enter key](image/pt-buttons-nav_enter.jpg)        |

[//]: # (TODO:)

[//]: # (phrase)

[//]: # (instrument)

[//]: # (table)

[//]: # (project)
