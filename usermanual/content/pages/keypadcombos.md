---
title: Controls & Moves
template: page
---

![labeled photo of Picotracker keymapping](image/pT-buttonMap.png)

The keyboard layout resembles a typical old console game controller

* Arrow keys
* NAV (RT)
* ALT (LT)
* ENTER (A)
* EDIT (B)
* Play (Start)

The NAV ([]) and ALT keys are modifier keys for the ENTER, EDIT and Arrow keys. They are designed to modify the opposite keys to their location, so ALT will modify the Arrow keys and NAV will modify A and B (there are exceptions).


## Basic Editing & Navigation
- ARROWs: In screen navigation.
- A: Insert Chain/Phrase/Note.
- A,A: Insert next unused Chain/Phrase/Instrument.
- RT+(B,A): Clone. This will overwrite the current Highlighted Item with a copy of itself using the next unused Item available.
- B+A: Cuts the current Highlighted Item .
- A+ARROWS: Updates Highlighted Item value.
  - A+UP/DOWN: +/- 0x10.
  - A+RIGHT/LEFT: +/- 1.
- B+ARROWS: Rapid Navigation.
  - B+UP/DOWN: Page up/down in Song Screen, Next/Previous Phrase in Current Chain in Phrase Screen. Navigation +/- 0x10 in Instrument/Table Screen.
  - B+LEFT/RIGHT: Next/Previous Channel in Chain/Phrase Screen. Navigation +/- 1 in Instrument/Table Screen. Switch between Song and Live Modes in Song Screen.
- LT+ARROWS: Navigate between the Screens.
- RT+UP/DOWN: Jump up/down to next populated row after a blank row (great for live mode entire row queuing!)

## Selections

a few ways to make a selection:

- RT+B: Starts selection mode with only the data at the cursor selected
- RT+B+B: Starts selection mode with the cursor's row selected
- RT+B+B+B: Starts selection mode with the entire screen selected

once a selection is started you can do a few more things:

- ARROWS: will make an existing selection bigger or smaller
- B: copy selection to buffer, or
- RT+A: cut current selection

And then:

- RT+A: paste the clipboard content at current location
