---
title: Controls & Moves
template: page
---

![labeled photo of Picotracker keymapping](image/pT-buttonMap.png)

The keyboard layout resembles a typical old console game controller (controller button names in brackets):

* `ARROW` keys
* `NAV` (RT)
* `ALT` (LT)
* `ENTER` (A)
* `EDIT` (B)
* `PLAY` (Start)

The `NAV` (`[]`) and `ALT` keys are modifier keys for the `ENTER`, `EDIT` and `ARROW` keys. They are designed to modify the opposite keys to their location, so `ALT` will modify the `ARROW` keys and `NAV` will modify `ENTER` and `EDIT` (though there are exceptions).


## Basic Editing & Navigation

- `ARROW`s: In screen navigation.
- `ENTER`: Insert Chain/Phrase/Note.
- `ENTER`, `ENTER`: Insert next unused Chain/Phrase/Instrument.
- `ALT`+(`EDIT`,`ENTER`): Clone. This will overwrite the current Highlighted Item with a copy of itself using the next unused Item available.
- `EDIT`+`ENTER`: Cuts the current Highlighted Item .
- `ENTER`+`ARROWS`: Updates Highlighted Item value.
  - `ENTER+UP/DOWN`: +/- 0x10.
  - `ENTER`+`RIGHT`/`LEFT`: +/- 1.
- `EDIT`+`ARROWS`: Rapid Navigation.
- `EDIT`+`UP`/`DOWN`: Page up/down in Song Screen, Next/Previous Phrase in Current Chain in Phrase Screen. Navigation +/- 0x10 in Instrument/Table Screen. In Import Screen, adjusts preview volume up/down.
- `EDIT`+`LEFT`/`RIGHT`: Next/Previous Channel in Chain/Phrase Screen. Navigation +/- 1 in Instrument/Table Screen. Switch between Song and Live Modes in Song Screen.
- `NAV`+`ARROWS`: Navigate between the Screens.
- `ALT`+`UP`/`DOWN`: Jump up/down to next populated row after a blank row (great for live mode entire row queuing!)

## Selections

A selection can be started in few different ways:

- `ALT`+`EDIT`: Starts selection mode with only the data at the cursor selected
- `ALT`+`EDIT`+`EDIT`: Starts selection mode with the cursor's row selected
- `ALT`+`EDIT`+`EDIT`+`EDIT`: Starts selection mode with the entire screen selected

Once a selection is started you can do a few more things:

- `ARROWS`: will make an existing selection bigger or smaller
- `EDIT`: will copy a selection to buffer, or
- `ALT`+`ENTER`: will cut the current selection

And then:

- `ALT`+`ENTER`: will paste the clipboard contents at the current cursor location
