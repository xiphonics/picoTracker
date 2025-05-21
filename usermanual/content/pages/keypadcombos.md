---
title: Controls & Moves
template: page
---

![labeled photo of Picotracker keymapping](image/pT-buttonMap.png)

The keyboard layout resembles a typical old console game controller (controller button names in brackets):

|   Key Names    |                         Image                          |
|:--------------:|:------------------------------------------------------:|
|  `ARROW` keys  | ![arrow keys](image/pt-buttons-arrows.jpg.resized.jpg) |
|   `NAV` (RT)   |  ![arrow keys](image/pt-buttons-nav.jpg.resized.jpg)   |
|   `ALT` (LT)   |  ![arrow keys](image/pt-buttons-alt.jpg.resized.jpg)   |
|  `ENTER` (A)   | ![arrow keys](image/pt-buttons-enter.jpg.resized.jpg)  |
|   `EDIT` (B)   |  ![arrow keys](image/pt-buttons-edit.jpg.resized.jpg)  |
| `PLAY` (Start) |  ![arrow keys](image/pt-buttons-play.jpg.resized.jpg)  |

The `NAV` (`[]`) and `ALT` keys are modifier keys for the `ENTER`, `EDIT` and `ARROW` keys. They are designed to modify
the opposite keys to their location, so `ALT` will modify the `ARROW` keys and `NAV` will modify `ENTER` and `EDIT` (
though there are exceptions).

## Basic Editing & Navigation

[//]: # (FIXME: is edit up/down redundant?)

[//]: # (FIXME: update image description!)

| Key Names                      |                              Image                               | Description                                                                                                                                                                                        |
|--------------------------------|:----------------------------------------------------------------:|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `ARROW`s                       |            ![arrow keys](image/pt-buttons-arrows.jpg)            | In screen navigation                                                                                                                                                                               |
| `ENTER`                        |             ![enter key](image/pt-buttons-enter.jpg)             | Insert Chain/Phrase/Note.                                                                                                                                                                          |
| `ENTER`, `ENTER`               |         ![enter key twice](image/pt-buttons-enter2.jpg)          | Insert next unused Chain/Phrase/Instrument.                                                                                                                                                        |
| `ALT` + (`EDIT`,`ENTER`)       | ![alt + (adit, enter) keys](image/pt-buttons-alt_edit_enter.jpg) | Clone. This will overwrite the current Highlighted Item with a copy of itself using the next unused Item available.                                                                                |
| `EDIT`+`ENTER`                 |      ![edit + enter keys](image/pt-buttons-edit_enter.jpg)       | Cuts the current Highlighted Item.                                                                                                                                                                 |
| `ENTER`+`ARROWS`               |     ![enter + arrow keys](image/pt-buttons-enter_arrows.jpg)     | Updates Highlighted Item value.                                                                                                                                                                    |
| `ENTER` + `UP` / `DOWN`        |    ![enter + up/down keys](image/pt-buttons-enter_updown.jpg)    | +/- 0x10                                                                                                                                                                                           |
| `ENTER` + `RIGHT` / `LEFT`     | ![enter + right/left keys](image/pt-buttons-enter_leftright.jpg) | +/- 1                                                                                                                                                                                              |
| `EDIT` + `ARROWS`              |     ![edit + arrows keys](image/pt-buttons-edit_arrows.jpg)      | Rapid Navigation                                                                                                                                                                                   |
| `EDIT` + `UP` / `DOWN`         |     ![edit + up/down keys](image/pt-buttons-edit_updown.jpg)     | Page up/down in Song Screen.<br> Next/Previous Phrase in Current Chain in Phrase Screen.<br> Navigation +/- 0x10 in Instrument/Table Screen.<br> In Import Screen, adjusts preview volume up/down. 
| `EDIT` + `LEFT` / `RIGHT`      |  ![edit + left/right keys](image/pt-buttons-edit_leftright.jpg)  | Next/Previous Channel in Chain/Phrase Screen.<br> Navigation +/- 1 in Instrument/Table Screen.<br> Switch between Song and Live Modes in Song Screen.                                              |
| `NAV` + `ARROWS`               |      ![nav + arrows keys](image/pt-buttons-nav_arrows.jpg)       | Navigate between the Screens.                                                                                                                                                                      |
| `ALT` + `UP` / `DOWN`          |      ![alt + up/down keys](image/pt-buttons-alt_updown.jpg)      | Jump up/down to next populated row after a blank row (great for live mode entire row queuing!)                                                                                                     |
| `EDIT` + `PLAY` (advance only) |       ![edit + play keys](image/pt-buttons-edit_play.jpg)        | Sample recording, only accessible when sequencer is stopped                                                                                                                                        |

## Selections

### Start Selection

A selection can be started in few different ways:

| Key Names                        |                           Image                           | Description                                                     |
|----------------------------------|:---------------------------------------------------------:|-----------------------------------------------------------------|
| `ALT` + `EDIT`                   |     ![alt + edit keys](image/pt-buttons-alt_edit.jpg)     | Starts selection mode with only the data at the cursor selected |
| `ALT` + `EDIT` + `EDIT`          | ![alt + edit twice keys](image/pt-buttons-alt_edit2.jpg)  | Starts selection mode with the cursor's row selected            |
| `ALT` + `EDIT` + `EDIT` + `EDIT` | ![alt + edit thrice keys](image/pt-buttons-alt_edit3.jpg) | Starts selection mode with the entire screen selected           |

### Use Selection

Once a selection is started you can do a few more things:

| Key Names       |                           Image                           | Description                                       |
|-----------------|:---------------------------------------------------------:|---------------------------------------------------|
| `ARROWS`        |  ![arrow keys](image/pt-buttons-arrows.jpg.resized.jpg)   | will make an existing selection bigger or smaller |
| `EDIT`          |   ![arrow keys](image/pt-buttons-edit.jpg.resized.jpg)    | will copy a selection to clipboard                |
| `ALT` + `ENTER` | ![arrow keys](image/pt-buttons-alt_enter.jpg.resized.jpg) | will cut the current selection                    |

### Use Clipboard

And then, after copying a selection to clipboard:

| Key Names       |                        Image                        | Description                                                      |
|-----------------|:---------------------------------------------------:|------------------------------------------------------------------|
| `ALT` + `ENTER` | ![alt + enter keys](image/pt-buttons-alt_enter.jpg) | will paste the clipboard contents at the current cursor location |
