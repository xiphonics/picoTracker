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

| Key Combination |                   Image                    | Function              |
|:---------------:|:------------------------------------------:|:----------------------|
|  `ARROW` keys   | ![arrow keys](image/pt-buttons-arrows.jpg) | Move cursor on screen |
|     `PLAY`      |   ![play key](image/pt-buttons-play.jpg)   | Start/stop playback   |

### NAV Modifier

| Key Combination |                       Image                        | Function                                               |
|:---------------:|:--------------------------------------------------:|:-------------------------------------------------------|
|   `NAV + UP`    |    ![nav + up key](image/pt-buttons-nav_up.jpg)    | Switch to Project view                                 |
|  `NAV + DOWN`   |  ![nav + down key](image/pt-buttons-nav_down.jpg)  | Switch to Mixer view                                   |
|  `NAV + RIGHT`  | ![nav + right key](image/pt-buttons-nav_right.jpg) | Switch to Chain view (if current position has a chain) |
|  `NAV + PLAY`   |  ![nav + play key](image/pt-buttons-nav_play.jpg)  | Stop playback of selected chain, once it finished      |
|  `NAV + ENTER`  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) | solo selected track                                    |
|  `NAV + EDIT`   | ![nav + edit keys](image/pt-buttons-nav_edit.jpg)  | mute selected track                                    |
|   `NAV + ALT`   |  ![nav + edit keys](image/pt-buttons-nav_alt.jpg)  | reset all muted & soloed tracks                        |

#### Solo & Mute

When `NAV` is released first, the (solo / mute) mode will be kept active until reset with `NAV + ALT`.

### ALT Modifier

| Key Combination            |                              Image                               | Function                                                                                              |
|:---------------------------|:----------------------------------------------------------------:|:------------------------------------------------------------------------------------------------------|
| `ALT + (UP / DOWN)`        |     ![alt + up/down keys](image/pt-buttons-alt_up_down.jpg)      | Jump up/down to next populated row after a blank row                                                  |
| `ALT + (LEFT / RIGHT)`     |  ![nav + alt + left/right](image/pt-buttons-alt_left_right.jpg)  | Nudge tempo down/up                                                                                   |
| `ALT + PLAY`               |        ![nav + alt + play](image/pt-buttons-alt_play.jpg)        | query current row for playbck                                                                         |
| `ALT + NAV`                |        ![nav + alt + enter](image/pt-buttons-nav_alt.jpg)        | reset all muted & soloed teacks                                                                       |
| `ALT + ENTER`              |          ![arrow keys](image/pt-buttons-alt_enter.jpg)           | cut the current cursor position if filled, paste otherwise                                            |
| `ALT + (EDIT + ENTER)`     | ![alt + (adit, enter) keys](image/pt-buttons-alt_edit_enter.jpg) | Clone: Overwrite current highlighted Item with a copy of itself using the next unused Item available. |
| `ALT + EDIT `              |        ![edit + alt keys](image/pt-buttons-alt_edit.jpg)         | start selection                                                                                       |
| `ALT + EDIT + EDIT`        |        ![edit + alt keys](image/pt-buttons-alt_edit2.jpg)        | start selection with row selected                                                                     |
| `ALT + EDIT + EDIT + EDIT` |        ![edit + alt keys](image/pt-buttons-alt_edit3.jpg)        | start selection with current screen selected                                                          |

#### Use Selection

Once a selection is started you can do a few more things:

| Key Names     |                     Image                      | Description                                                |
|---------------|:----------------------------------------------:|------------------------------------------------------------|
| `ARROWS`      |   ![arrow keys](image/pt-buttons-arrows.jpg)   | change selection                                           |
| `EDIT`        |    ![arrow keys](image/pt-buttons-edit.jpg)    | copy selection to clipboard                                |
| `PLAY`        |    ![arrow keys](image/pt-buttons-play.jpg)    | queue selection for playback                               |
| `NAV + PLAY`  |  ![arrow keys](image/pt-buttons-nav_play.jpg)  | stop                                                       |
| `ALT + ENTER` | ![arrow keys](image/pt-buttons-alt_enter.jpg)  | cut the current cursor position if filled, paste otherwise |
| `NAV + ENTER` | ![nav + enter](image/pt-buttons-nav_enter.jpg) | switch solo mode                                           |
| `ALT + NAV`   |  ![nav + enter](image/pt-buttons-nav_alt.jpg)  | unmute all                                                 |

### EDIT Modifier

| Key Combination                |                             Image                              | Function                                                    |
|:-------------------------------|:--------------------------------------------------------------:|:------------------------------------------------------------|
| `EDIT + ENTER`                 |     ![edit + enter keys](image/pt-buttons-edit_enter.jpg)      | Cuts the current Highlighted Item.                          |
| `ALT + EDIT`                   |       ![edit + alt keys](image/pt-buttons-alt_edit.jpg)        | start selection                                             |
| `EDIT + PLAY`                  |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       | Start immediate playback                                    |
| `EDIT + (UP / DOWN)`           |    ![edit + up/down keys](image/pt-buttons-edit_updown.jpg)    | Page up/down in Song Screen.                                |
| `EDIT + (LEFT / RIGHT)`        | ![edit + left/right keys](image/pt-buttons-edit_leftright.jpg) | Switch between Song and Live Mode                           |
| `EDIT` + `PLAY` (advance only) |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       | Sample recording, only accessible when sequencer is stopped |

### ENTER Key Combinations

| Key Combination          |                               Image                               | Function                                                   |
|:-------------------------|:-----------------------------------------------------------------:|:-----------------------------------------------------------|
| `ENTER`                  |             ![enter key](image/pt-buttons-enter.jpg)              | insert if cursor position is empty                         |
| `ENTER, ENTER`           |          ![enter key twice](image/pt-buttons-enter2.jpg)          | Insert next unused chain                                   |
| `ENTER + (UP / DOWN)`    |     ![enter + arrow keys](image/pt-buttons-enter_up_down.jpg)     | Change chain at cursor position by 0x10                    |
| `ENTER + (RIGHT / LEFT)` | ![enter + right/left keys](image/pt-buttons-enter_left_right.jpg) | Change chain at cursor position by 0x01                    |
| `ALT + ENTER`            |        ![enter + alt keys](image/pt-buttons-alt_enter.jpg)        | cut the current cursor position if filled, paste otherwise |

[//]: # (TODO:)

[//]: # (chain)

[//]: # (phrase)

[//]: # (instrument)

[//]: # (table)

[//]: # (project)
