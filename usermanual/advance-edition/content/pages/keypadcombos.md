---
title: Controls & Moves
template: page
---

![labeled photo of Picotracker keymapping](image/advance-keymap-small.png)

The keyboard layout resembles a typical console game controller (controller button names in brackets):

| <span class="minikeys">ARROW</span> keys                               | <span class="minikeys">NAV</span> (RT)                              | <span class="minikeys">ALT</span> (LT)                              | <span class="minikeys">ENTER</span> (A)                               | <span class="minikeys">EDIT</span> (B)                               | <span class="minikeys">PLAY</span> (Start)                           |
|:-------------------------------------------|:----------------------------------------|:----------------------------------------|:------------------------------------------|:-----------------------------------------|:-----------------------------------------|
| ![arrow keys](image/pt-buttons-arrows.jpg) | ![arrow keys](image/pt-buttons-nav.jpg) | ![arrow keys](image/pt-buttons-alt.jpg) | ![arrow keys](image/pt-buttons-enter.jpg) | ![arrow keys](image/pt-buttons-edit.jpg) | ![arrow keys](image/pt-buttons-play.jpg) |

The <span class="minikeys">NAV</span> and <span class="minikeys">ALT</span> keys are modifier keys for the <span class="minikeys">ENTER</span>, <span class="minikeys">EDIT</span> and <span class="minikeys">ARROW</span> keys. They are designed to modify
the opposite keys to their location, so <span class="minikeys">ALT</span> will modify the <span class="minikeys">ARROW</span> keys and <span class="minikeys">NAV</span> will modify <span class="minikeys">ENTER</span> and <span class="minikeys">EDIT</span> (
though there are exceptions).

On tracker screens where <span class="minikeys">PLAY</span> controls playback, holding <span class="minikeys">PLAY</span> for about one second triggers a panic stop. This immediately stops the sequencer and **cuts any currently sounding audio**, which is useful if a note, effect tail, or external MIDI sound gets stuck. A short press of <span class="minikeys">PLAY</span> still performs the normal start/stop or queue action when you release the key.

### Remark about navigation between screens

Screens become more granular as you move from left to right.

Navigating to the next right screen is only possible if the current cursor position is on a field with a filled value, eg. to move to the chain screen from the Song screen you need to have a chain number on the cursors current position and not an empty position which are denoted with 2 dashes (`--`).


## Song View

### No Modifier

| Function                              | Key Combination |                   Image                    |
|:--------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                 |  <span class="minikeys">ARROW</span> keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback                   |     <span class="minikeys">PLAY</span>      | ![play key](image/pt-buttons-play.jpg)   |
| Panic stop sounding audio             |  hold <span class="minikeys">PLAY</span>    | ![play key](image/pt-buttons-play.jpg)   |
| Enter last used value or start with 0 |     <span class="minikeys">ENTER</span>     | ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function                                               | Key Combination |                       Image                        |
|:-------------------------------------------------------|:---------------:|:--------------------------------------------------:|
| Switch to Project view                                 | <span class="minikeys">NAV</span> + <span class="minikeys">UP</span>    |    ![nav + up key](image/pt-buttons-nav_up.jpg)    |
| Switch to Mixer view                                   | <span class="minikeys">NAV</span> + <span class="minikeys">DOWN</span>   |  ![nav + down key](image/pt-buttons-nav_down.jpg)  |
| Switch to Chain view (if current position has a chain) | <span class="minikeys">NAV</span> + <span class="minikeys">RIGHT</span>  | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| Stop playback of selected chain, once it finished      | <span class="minikeys">NAV</span> + <span class="minikeys">PLAY</span>   |  ![nav + play key](image/pt-buttons-nav_play.jpg)  |
| Solo selected track                                    | <span class="minikeys">NAV</span> + <span class="minikeys">ENTER</span>  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) |
| Mute selected track                                    | <span class="minikeys">NAV</span> + <span class="minikeys">EDIT</span>   | ![nav + edit keys](image/pt-buttons-nav_edit.jpg)  |
| Reset all muted & solo-ed tracks                       | <span class="minikeys">NAV</span> + <span class="minikeys">ALT</span>   |  ![nav + edit keys](image/pt-buttons-nav_alt.jpg)  |

#### Solo & Mute

When <span class="minikeys">NAV</span> is released first, the (solo / mute) mode will be kept active (toggled).
Pressing the same key combination but releasing <span class="minikeys">NAV</span> first will reset the (solo / mute) mode of the currently selected
track.

To reset all muted and soloed tracks, press <span class="minikeys">NAV</span> + <span class="minikeys">ALT</span>.

### ALT Modifier

| Function                                                                                              |      Key Combination       |                              Image                              |
|:------------------------------------------------------------------------------------------------------|:--------------------------:|:---------------------------------------------------------------:|
| Jump up/down to next populated row after a blank row                                                  | <span class="minikeys">ALT</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)     |     ![alt + up/down keys](image/pt-buttons-alt_up_down.jpg)     |
| Nudge tempo down/up                                                                                   | <span class="minikeys">ALT</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>)   | ![nav + alt + left/right](image/pt-buttons-alt_left_right.jpg)  |
| Start/stop song, or queue current row for playback in Live mode                                     | <span class="minikeys">ALT</span> + <span class="minikeys">PLAY</span> |   ![nav + alt + play](image/pt-buttons-alt_play.jpg)        |
| Reset all muted & soloed tracks                                                                       | <span class="minikeys">ALT</span> + <span class="minikeys">NAV</span>         |       ![nav + alt + enter](image/pt-buttons-nav_alt.jpg)        |
| Cut the current cursor position if filled, paste otherwise                                            | <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>        |          ![arrow keys](image/pt-buttons-alt_enter.jpg)          |
| Clone: Overwrite current highlighted Item with a copy of itself using the next unused Item available. | <span class="minikeys">ALT</span> + (<span class="minikeys">EDIT</span>, <span class="minikeys">ENTER</span>)    | ![alt + edit + enter keys](image/pt-buttons-alt_edit_enter.jpg) |
| Start selection                                                                                       | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>        |        ![edit + alt keys](image/pt-buttons-alt_edit.jpg)        |
| Selection with current row selected                                                                   | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span>     |       ![edit + alt keys](image/pt-buttons-alt_edit2.jpg)        |
| Selection with current screen selected                                                          | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span> |       ![edit + alt keys](image/pt-buttons-alt_edit3.jpg)        |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                             | Key Combination |                     Image                     |
|-----------------------------------------|:---------------:|:---------------------------------------------:|
| Change selection                        | <span class="minikeys">ARROWS</span>     |  ![arrow keys](image/pt-buttons-arrows.jpg)   |
| Increase selection to full row / screen | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>   | ![nav + enter](image/pt-buttons-alt_edit.jpg) |
| Copy selection to clipboard             | <span class="minikeys">EDIT</span>      |   ![arrow keys](image/pt-buttons-edit.jpg)    |
| Cut selection to clipboard              | <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>  | ![arrow keys](image/pt-buttons-alt_enter.jpg) |
| Resample selection                      | <span class="minikeys">ENTER</span> + <span class="minikeys">PLAY</span>  |    ![play key](image/pt-buttons-play.jpg)     |

### EDIT Modifier

| Function                                                                     |     Key Combination     |                             Image                              |
|:-----------------------------------------------------------------------------|:-----------------------:|:--------------------------------------------------------------:|
| Cuts the current Highlighted Item.                                           | <span class="minikeys">EDIT</span> + <span class="minikeys">ENTER</span>      | ![edit + enter keys](image/pt-buttons-edit_enter.jpg)      |
| Start selection                                                              | <span class="minikeys">EDIT</span> + <span class="minikeys">ALT</span>        | ![edit + alt keys](image/pt-buttons-alt_edit.jpg)        |
| Page up/down in Song Screen.                                                 | <span class="minikeys">EDIT</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)   |    ![edit + up/down keys](image/pt-buttons-edit_updown.jpg)    |
| Switch between Song and Live Mode                                            | <span class="minikeys">EDIT</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>) | ![edit + left/right keys](image/pt-buttons-edit_leftright.jpg) |
| Mute selected track                                                          | <span class="minikeys">EDIT</span> + <span class="minikeys">NAV</span>       |       ![nav + edit keys](image/pt-buttons-nav_edit.jpg)        |
| Sample Recording, only accessible when sequencer is stopped |      <span class="minikeys">EDIT</span> + <span class="minikeys">PLAY</span>      |      ![edit + play keys](image/pt-buttons-edit_play.jpg)       |

### ENTER Key Combinations

| Function                                                   |     Key Combination      |                               Image                               |
|:-----------------------------------------------------------|:------------------------:|:-----------------------------------------------------------------:|
| Insert if cursor position is empty                         | <span class="minikeys">ENTER</span>          |             ![enter key](image/pt-buttons-enter.jpg)              |
| Insert next unused chain                                   | <span class="minikeys">ENTER</span>, <span class="minikeys">ENTER</span>      |          ![enter key twice](image/pt-buttons-enter2.jpg)          |
| Cut the current cursor position if filled, paste otherwise | <span class="minikeys">ENTER</span> + <span class="minikeys">ALT</span>       |           ![arrow keys](image/pt-buttons-alt_enter.jpg)           |
| Change chain at cursor position by 0x10                    | <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)   |     ![enter + arrow keys](image/pt-buttons-enter_up_down.jpg)     |
| Change chain at cursor position by 0x01                    | <span class="minikeys">ENTER</span> + (<span class="minikeys">RIGHT</span> / <span class="minikeys">LEFT</span>) | ![enter + right/left keys](image/pt-buttons-enter_left_right.jpg) |
| Solo selected track                                        | <span class="minikeys">ENTER</span> + <span class="minikeys">NAV</span>       |        ![nav + enter key](image/pt-buttons-nav_enter.jpg)         |

## Chain View

### No Modifier

| Function                               | Key Combination |                   Image                    |
|:---------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                  | <span class="minikeys">ARROW</span> keys | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback of current chain   | <span class="minikeys">PLAY</span>      | ![play key](image/pt-buttons-play.jpg)   |
| Enter last used phrase or start with 0 | <span class="minikeys">ENTER</span>     | ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function              | Key Combination |                       Image                        |
|:----------------------|:---------------:|:--------------------------------------------------:|
| Play chain in song    | <span class="minikeys">NAV</span> + <span class="minikeys">PLAY</span>   | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| Unmute all            | <span class="minikeys">NAV</span> + <span class="minikeys">ALT</span>   | ![nav + alt key](image/pt-buttons-nav_alt.jpg)   |
| Toggle mute           | <span class="minikeys">NAV</span> + <span class="minikeys">EDIT</span>   | ![nav + edit key](image/pt-buttons-nav_edit.jpg)  |
| Switch solo mode      | <span class="minikeys">NAV</span> + <span class="minikeys">ENTER</span>  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) |

### ALT Modifier

| Function               |    Key Combination    |                             Image                              |
|:-----------------------|:---------------------:|:--------------------------------------------------------------:|
| Unmute all             | <span class="minikeys">ALT</span> + <span class="minikeys">NAV</span>      |         ![alt + nav key](image/pt-buttons-nav_alt.jpg)         |
| Clone current position | <span class="minikeys">ALT</span> + (<span class="minikeys">EDIT</span>, <span class="minikeys">ENTER</span>) | ![alt + edit + enter key](image/pt-buttons-alt_edit_enter.jpg) |
| Paste clipboard        | <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>     |       ![alt + enter key](image/pt-buttons-alt_enter.jpg)       |
| Start selection        | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>      |        ![alt + edit key](image/pt-buttons-alt_edit.jpg)        |

### EDIT Modifier

| Function                                                                     |      Key Combination       |                                Image                                 |
|:-----------------------------------------------------------------------------|:--------------------------:|:--------------------------------------------------------------------:|
| Warp to (previous / next) channel                                            | <span class="minikeys">EDIT</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>)   | ![edit + horizontal arrow keys](image/pt-buttons-edit_leftright.jpg) |
| Warp to (previous / next) chain of current channel                           | <span class="minikeys">EDIT</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)    |   ![edit + vertical arrow keys](image/pt-buttons-edit_updown.jpg)    |
| Cut current position into clipboard                                          | <span class="minikeys">EDIT</span> + <span class="minikeys">ENTER</span>       |         ![edit + enter key](image/pt-buttons-edit_enter.jpg)         |
| Clone current position                                                       | <span class="minikeys">EDIT</span> + <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>    |    ![edit + alt + enter key](image/pt-buttons-alt_edit_enter.jpg)    |
| Toggle mute                                                                  | <span class="minikeys">EDIT</span> + <span class="minikeys">NAV</span>        |           ![edit + nav key](image/pt-buttons-nav_edit.jpg)           |
| Start selection                                                              | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>        |           ![alt + edit key](image/pt-buttons-alt_edit.jpg)           |
| Selection with row selected                                                  | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span>     |       ![alt + edit twice keys](image/pt-buttons-alt_edit2.jpg)       |
| Selection with current screen selected                                       | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span> |      ![alt + edit thrice keys](image/pt-buttons-alt_edit3.jpg)       |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                         |     Key Combination      |                                    Image                                    |
|-------------------------------------|:------------------------:|:---------------------------------------------------------------------------:|
| Change selection                    | <span class="minikeys">ARROWS</span>         |                 ![arrow keys](image/pt-buttons-arrows.jpg)                  |
| Copy selection to clipboard         | <span class="minikeys">EDIT</span>          |                   ![edit key](image/pt-buttons-edit.jpg)                    |
| Toggle mute                         | <span class="minikeys">EDIT</span> + <span class="minikeys">NAV</span>       |              ![edit + nav key](image/pt-buttons-nav_edit.jpg)               |
| Update selection values by +/- 0x10 | <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> + <span class="minikeys">DOWN</span>)   |     ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)      |
| Update selection values by +/- 0x01 | <span class="minikeys">ENTER</span> + (<span class="minikeys">LEFT</span> + <span class="minikeys">RIGHT</span>) | ![enter key + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| Cut the current selection           | <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>       |             ![alt + enter key](image/pt-buttons-alt_enter.jpg)              |
| Resample selection                  | <span class="minikeys">ENTER</span> + <span class="minikeys">PLAY</span>      |                   ![play key](image/pt-buttons-play.jpg)                    |
| Switch solo mode                    | <span class="minikeys">NAV</span> + <span class="minikeys">ENTER</span>       |             ![nav + enter key](image/pt-buttons-nav_enter.jpg)              |
| Play chain                          | <span class="minikeys">PLAY</span>          |                   ![play key](image/pt-buttons-play.jpg)                    |
| Play chain in song                  | <span class="minikeys">NAV</span> + <span class="minikeys">PLAY</span>       |              ![nav + play key](image/pt-buttons-nav_play.jpg)               |
| Unmute all                          | <span class="minikeys">ALT</span> + <span class="minikeys">NAV</span>        |               ![alt + nav key](image/pt-buttons-nav_alt.jpg)                |

### ENTER Key Combinations

| Function                        |     Key Combination      |                                  Image                                  |
|:--------------------------------|:------------------------:|:-----------------------------------------------------------------------:|
| Update cursor value by +/- 0x10 | <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)   |   ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| Update cursor value by +/- 0x01 | <span class="minikeys">ENTER</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>) | ![enter + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| Paste clipboard                 | <span class="minikeys">ENTER</span> + <span class="minikeys">ALT</span>       |           ![enter + alt key](image/pt-buttons-alt_enter.jpg)            |
| Switch solo mode                | <span class="minikeys">ENTER</span> + <span class="minikeys">NAV</span>       |           ![enter + nav key](image/pt-buttons-nav_enter.jpg)            |

## Phrase View

### No Modifier

| Function                                                                                   | Key Combination |                   Image                    |
|:--------------------------------------------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                         | <span class="minikeys">ARROW</span> keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback of current phrase         | <span class="minikeys">PLAY</span>      |   ![play key](image/pt-buttons-play.jpg)   |
| enter last used value or start with 0         | <span class="minikeys">ENTER</span>     |  ![enter key](image/pt-buttons-enter.jpg)  |
| Insert first unused instrument/table number   | <span class="minikeys">ENTER</span>,<span class="minikeys">ENTER</span>  |  ![enter key](image/pt-buttons-enter.jpg)  |
| In note / instrument column: audition note    |  <span class="minikeys">ENTER</span> held   |  ![enter key](image/pt-buttons-enter.jpg)  |


| Function                  | Key Combination |                       Image                        |
|:--------------------------|:---------------:|:--------------------------------------------------:|
| Play phrase in song       | <span class="minikeys">NAV</span> + <span class="minikeys">PLAY</span>   | ![nav + play key](image/pt-buttons-nav_play.jpg)  |
| Unmute all                | <span class="minikeys">NAV</span> + <span class="minikeys">ALT</span>   | ![nav + alt key](image/pt-buttons-nav_alt.jpg)   |
| Switch mute mode          | <span class="minikeys">NAV</span> + <span class="minikeys">EDIT</span>   | ![nav + edit key](image/pt-buttons-nav_edit.jpg)  |
| Switch solo mode          | <span class="minikeys">NAV</span> + <span class="minikeys">ENTER</span>  | ![nav + enter key](image/pt-buttons-nav_enter.jpg) |

### ALT Modifier

| Function                                                                        |   Key Combination    |                             Image                              |
|:--------------------------------------------------------------------------------|:--------------------:|:--------------------------------------------------------------:|
| Unmute all                                                                      | <span class="minikeys">ALT</span> + <span class="minikeys">NAV</span>      |         ![alt + nav key](image/pt-buttons-nav_alt.jpg)         |
| Paste clipboard                                                                 | <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>     |       ![alt + enter key](image/pt-buttons-alt_enter.jpg)       |
| Start selection                                                                 | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>     |        ![alt + edit key](image/pt-buttons-alt_edit.jpg)        |
| Clone current instrument/table (cursor in note / instrument column)             | <span class="minikeys">ALT</span> + (<span class="minikeys">EDIT</span>, <span class="minikeys">ENTER</span>) | ![alt + edit + enter key](image/pt-buttons-alt_edit_enter.jpg) |

### EDIT Modifier

| Function                                                                     |      Key Combination       |                                Image                                 |
|:-----------------------------------------------------------------------------|:--------------------------:|:--------------------------------------------------------------------:|
| Warp to (previous / next) track                                              | <span class="minikeys">EDIT</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>)   | ![edit + horizontal arrow keys](image/pt-buttons-edit_leftright.jpg)|
| Warp to (previous / next) phrase in chain                                    | <span class="minikeys">EDIT</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)    |   ![edit + vertical arrow keys](image/pt-buttons-edit_updown.jpg)  |
| Cut current position into clipboard                                          | <span class="minikeys">EDIT</span> + <span class="minikeys">ENTER</span>       |         ![edit + enter key](image/pt-buttons-edit_enter.jpg)         |
| Toggle mute                                                                  | <span class="minikeys">EDIT</span> + <span class="minikeys">NAV</span>        |           ![edit + nav key](image/pt-buttons-nav_edit.jpg)           |
| Start selection                                                              | <span class="minikeys">EDIT</span> + <span class="minikeys">ALT</span>        |           ![alt + edit key](image/pt-buttons-alt_edit.jpg)           |
| Start selection with row selected                                            | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span>     |       ![alt + edit twice keys](image/pt-buttons-alt_edit2.jpg)     |
| Start selection with current screen selected                                 | <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span> + <span class="minikeys">EDIT</span> |      ![alt + edit thrice keys](image/pt-buttons-alt_edit3.jpg)       |

#### Use Selection

Once a selection is started you can do a few more things:

| Description                             |     Key Combination      |                                  Image                                   |
|-----------------------------------------|:------------------------:|:------------------------------------------------------------------------:|
| Change selection                        |         <span class="minikeys">ARROWS</span>         |                ![arrow keys](image/pt-buttons-arrows.jpg)                |
| Increase selection to full row / screen |       <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>       |              ![nav + enter](image/pt-buttons-alt_edit.jpg)               |
| Copy selection to clipboard             |          <span class="minikeys">EDIT</span>          |                  ![edit key](image/pt-buttons-edit.jpg)                  |
| Cut the current selection               |      <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>       |            ![alt + enter key](image/pt-buttons-alt_enter.jpg)            |
| Update selection values by +/- 0x10     |  <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> + <span class="minikeys">DOWN</span>)   |    ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| Update selection values by +/- 0x01     | <span class="minikeys">ENTER</span> + (<span class="minikeys">LEFT</span> + <span class="minikeys">RIGHT</span>) | ![enter  + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| Resample selection                      |      <span class="minikeys">ENTER</span> + <span class="minikeys">PLAY</span>      |                 ![play key](image/pt-buttons-play.jpg)                  |

### ENTER Key Combinations

| Function                        |     Key Combination      |                                  Image                                  |
|:--------------------------------|:------------------------:|:-----------------------------------------------------------------------:|
| update cursor value by +/- 0x10 | <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)   |   ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| update cursor value by +/- 0x01 | <span class="minikeys">ENTER</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>) | ![enter + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| paste clipboard                 | <span class="minikeys">ENTER</span> + <span class="minikeys">ALT</span>       | ![enter + alt key](image/pt-buttons-alt_enter.jpg)            |
| switch solo mode                | <span class="minikeys">ENTER</span> + <span class="minikeys">NAV</span>       | ![enter + nav key](image/pt-buttons-nav_enter.jpg)            |

## Instrument View

### No Modifier

| Function              | Key Combination |                   Image                    |
|:----------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen |  <span class="minikeys">ARROW</span> keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback   |     <span class="minikeys">PLAY</span>      | ![play key](image/pt-buttons-play.jpg)   |

### ENTER Key

| Function                                                  | Key Combination |                   Image                   |
|:----------------------------------------------------------|:---------------:|:-----------------------------------------:|
| Import sample from samplelib (double-tap on sample field) | <span class="minikeys">ENTER</span>, <span class="minikeys">ENTER</span>  | ![enter key](image/pt-buttons-enter2.jpg) |
| Get next available table (on table field)                 | <span class="minikeys">ENTER</span>     | ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function                                             | Key Combination |                       Image                        |
|:-----------------------------------------------------|:---------------:|:--------------------------------------------------:|
| Switch to Phrase view                                |  <span class="minikeys">NAV</span> + <span class="minikeys">LEFT</span>   | ![nav + left key](image/pt-buttons-nav_left.jpg)  |
| Switch to Voice view (Sample instrument)             |  <span class="minikeys">NAV</span> + <span class="minikeys">RIGHT</span>  | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| Switch to Table view (if instrument has table)       |  <span class="minikeys">NAV</span> + <span class="minikeys">DOWN</span>   | ![nav + down key](image/pt-buttons-nav_down.jpg)  |
| Start playback (phrase mode)                         |  <span class="minikeys">NAV</span> + <span class="minikeys">PLAY</span>   | ![nav + play key](image/pt-buttons-nav_play.jpg)  |

### ALT Modifier

| Function                                                                                         | Key Combination |                         Image                          |
|:-------------------------------------------------------------------------------------------------|:---------------:|:------------------------------------------------------:|
| Audition current audio instrument in the Instrument or Voice view (hold; works during playback) | <span class="minikeys">ALT</span> + <span class="minikeys">PLAY</span> | ![alt + play key](image/pt-buttons-alt_play.jpg) |

### EDIT Modifier

| Function                                                                     |     Key Combination     |                               Image                               |
|:-----------------------------------------------------------------------------|:-----------------------:|:-----------------------------------------------------------------:|
| Previous/Next instrument (-1/+1)                                             | <span class="minikeys">EDIT</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>) | ![edit + (left / right) key](image/pt-buttons-edit_leftright.jpg) |
| Previous/Next instrument (-16/+16)                                           | <span class="minikeys">EDIT</span> + (<span class="minikeys">DOWN</span> / <span class="minikeys">UP</span>)   |       ![edit + down key](image/pt-buttons-edit_updown.jpg)        |
| Cut/purge instrument (sample) or clear table                                 | <span class="minikeys">EDIT</span> + <span class="minikeys">ENTER</span>      |       ![edit + enter key](image/pt-buttons-edit_enter.jpg)        |
| Clone current table                                                          | <span class="minikeys">ALT</span> + (<span class="minikeys">EDIT</span>, <span class="minikeys">ENTER</span>)   |      ![edit + alt key](image/pt-buttons-alt_edit_enter.jpg)       |

## Import View

### No Modifier

| Function                       | Key Combination |                         Image                         |
|:-------------------------------|:---------------:|:-----------------------------------------------------:|
| Move up/down in file list      | <span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>   |  ![arrow keys](image/pt-buttons-arrows-up_down.jpg)   |
| Toggle selected button         | <span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>  | ![arrow keys](image/pt-buttons-arrows-left_right.jpg) |
| Preview selected sample (hold) | <span class="minikeys">PLAY</span>      |        ![play key](image/pt-buttons-play.jpg)         |
| Navigate into directory        | <span class="minikeys">ENTER</span>     |       ![enter key](image/pt-buttons-enter.jpg)        |

### ENTER Key

**In File Browser Mode:**

| Function      | Key Combination                       |                  Image                   |
|:--------------|:--------------------------------------|:----------------------------------------:|
| Import sample | <span class="minikeys">ENTER</span> (with Import button selected) | ![enter key](image/pt-buttons-enter.jpg) |
| Edit sample   | <span class="minikeys">ENTER</span> (with Edit button selected)   | ![enter key](image/pt-buttons-enter.jpg) |

**In Project Pool Mode:**

| Function                                    | Key Combination                       |                  Image                   |
|:--------------------------------------------|:--------------------------------------|:----------------------------------------:|
| Edit sample                                 | <span class="minikeys">ENTER</span> (with Edit button selected)   | ![enter key](image/pt-buttons-enter.jpg) |
| Remove sample from project                  | <span class="minikeys">ENTER</span> (with Remove button selected) | ![enter key](image/pt-buttons-enter.jpg) |

### NAV Modifier

| Function                                       | Key Combination |                      Image                       |
|:-----------------------------------------------|:---------------:|:------------------------------------------------:|
| Return to source view (Phrase or Instrument)   |  <span class="minikeys">NAV</span> + <span class="minikeys">LEFT</span>   | ![nav + left key](image/pt-buttons-nav_left.jpg) |
| Go to parent directory                         | <span class="minikeys">NAV</span> + <span class="minikeys">UP</span>     | ![nav + up key](image/pt-buttons-nav_up.jpg)     |
| Toggle between sample library and project pool |  <span class="minikeys">NAV</span> + <span class="minikeys">EDIT</span>   | ![nav + edit key](image/pt-buttons-nav_edit.jpg) |

### EDIT Modifier

| Function                           |   Key Combination    |                            Image                             |
|:-----------------------------------|:--------------------:|:------------------------------------------------------------:|
| Increase / Decrease preview volume | <span class="minikeys">EDIT</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>) | ![edit + (up / down) key](image/pt-buttons-edit_up_down.jpg) |

### ALT Modifier

| Function      | Key Combination |                      Image                       |
|:--------------|:---------------:|:------------------------------------------------:|
| Import sample | <span class="minikeys">ALT</span> + <span class="minikeys">PLAY</span>   | ![alt + play key](image/pt-buttons-alt_play.jpg) |

## Sample Editor View

### No Modifier

| Function              | Key Combination |                   Image                    |
|:----------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen | <span class="minikeys">ARROW</span> keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Preview sample (hold) | <span class="minikeys">PLAY</span>      |   ![play key](image/pt-buttons-play.jpg)   |

### ENTER Key

| Function                        |                        Key Combination                         |                                  Image                                  |
|:--------------------------------|:--------------------------------------------------------------:|:-----------------------------------------------------------------------:|
| Update cursor value             | <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>) (with start, end or name field selected) |   ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| Update cursor position in field | <span class="minikeys">ENTER</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>)                    | ![enter + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| Save sample                     | <span class="minikeys">ENTER</span> (with Save button selected)               |                ![enter key](image/pt-buttons-enter.jpg)                 |

### EDIT Key

| Function                            |          Key Combination          |                 Image                  |
|:------------------------------------|:---------------------------------:|:--------------------------------------:|
| delete character at cursor position | <span class="minikeys">EDIT</span> (with name field selected) | ![edit key](image/pt-buttons-edit.jpg) |

### NAV Modifier

| Function                 | Key Combination |                      Image                       |
|:-------------------------|:---------------:|:------------------------------------------------:|
| Return to sample browser |  <span class="minikeys">NAV</span> + <span class="minikeys">LEFT</span>   | ![nav + left key](image/pt-buttons-nav_left.jpg) |

## Table View

### No Modifier

| Function                                    | Key Combination |                   Image                    |
|:--------------------------------------------|:---------------:|:------------------------------------------:|
| Move cursor on screen                       | <span class="minikeys">ARROW</span> keys   | ![arrow keys](image/pt-buttons-arrows.jpg) |
| Start/stop playback of current phrase       | <span class="minikeys">PLAY</span>      |   ![play key](image/pt-buttons-play.jpg)   |
| Paste last used command (in command column) | <span class="minikeys">ENTER</span>     |  ![enter key](image/pt-buttons-enter.jpg)  |

### NAV Modifier

| Function                                                             | Key Combination |                       Image                        |
|:---------------------------------------------------------------------|:---------------:|:--------------------------------------------------:|
| Switch to Phrase View (from Table1) or Instrument View (from Table2) | <span class="minikeys">NAV</span> + <span class="minikeys">UP</span>    |    ![nav + up key](image/pt-buttons-nav_up.jpg)    |
| Switch to Table View (from Table2)                                   | <span class="minikeys">NAV</span> + <span class="minikeys">LEFT</span>   |  ![nav + left key](image/pt-buttons-nav_left.jpg)  |
| Switch to Table2 View (from Table1)                                  | <span class="minikeys">NAV</span> + <span class="minikeys">RIGHT</span>  | ![nav + right key](image/pt-buttons-nav_right.jpg) |
| Play phrase in song context                                          | <span class="minikeys">NAV</span> + <span class="minikeys">PLAY</span>   |  ![nav + play key](image/pt-buttons-nav_play.jpg)  |

### ALT Modifier

| Function        | Key Combination |                       Image                        |
|:----------------|:---------------:|:--------------------------------------------------:|
| Paste clipboard |  <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>  | ![alt + enter key](image/pt-buttons-alt_enter.jpg) |

### EDIT Modifier

| Function                                                        |     Key Combination     |                               Image                               |
|:----------------------------------------------------------------|:-----------------------:|:-----------------------------------------------------------------:|
| Warp to previous/next table (-1/+1)                             | <span class="minikeys">EDIT</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>) | ![edit + (left / right) key](image/pt-buttons-edit_leftright.jpg) |
| Warp to previous/next table (-16/+16)                           | <span class="minikeys">EDIT</span> + (<span class="minikeys">DOWN</span> / <span class="minikeys">UP</span>)   |       ![edit + down key](image/pt-buttons-edit_updown.jpg)        |
| Cut current position into clipboard                             | <span class="minikeys">EDIT</span> + <span class="minikeys">ENTER</span>      |       ![edit + enter key](image/pt-buttons-edit_enter.jpg)        |
| Start selection mode                                            | <span class="minikeys">EDIT</span> + <span class="minikeys">ALT</span>       |         ![alt + edit key](image/pt-buttons-alt_edit.jpg)          |

### Use Selection

Once a selection is started you can do a few more things:

| Function                                | Key Combination |                       Image                        |
|:----------------------------------------|:---------------:|:--------------------------------------------------:|
| Change selection                        |  <span class="minikeys">ARROW</span> keys   |     ![arrow keys](image/pt-buttons-arrows.jpg)     |
| Increase selection to full row / screen |  <span class="minikeys">ALT</span> + <span class="minikeys">EDIT</span>   |  ![alt + edit key](image/pt-buttons-alt_edit.jpg)  |
| Copy selection to clipboard             |  <span class="minikeys">EDIT</span>      |       ![edit key](image/pt-buttons-edit.jpg)       |
| Cut the current selection               |  <span class="minikeys">ALT</span> + <span class="minikeys">ENTER</span>  | ![enter + alt key](image/pt-buttons-alt_enter.jpg) |

### ENTER Modifier

| Function                        |     Key Combination      |                                  Image                                  |
|:--------------------------------|:------------------------:|:-----------------------------------------------------------------------:|
| Update cursor value by +/- 0x10 | <span class="minikeys">ENTER</span> + (<span class="minikeys">UP</span> / <span class="minikeys">DOWN</span>)   |   ![enter + vertical arrow keys](image/pt-buttons-enter_up_down.jpg)    |
| Update cursor value by +/- 0x01 | <span class="minikeys">ENTER</span> + (<span class="minikeys">LEFT</span> / <span class="minikeys">RIGHT</span>) | ![enter + horizontal arrow keys](image/pt-buttons-enter_left_right.jpg) |
| Paste clipboard                 | <span class="minikeys">ENTER</span> + <span class="minikeys">ALT</span>       |           ![enter + alt key](image/pt-buttons-alt_enter.jpg)            |
