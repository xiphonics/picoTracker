---
title: Phrase Screen
template: page
---

![screen capture of Phrase screen](image/phrase-screen-small.png)

- The top of the phrase screen displays the name of the instrument under your cursor.
- The seven columns of the phrase screen, from left to right are: 
  * row counter
  * note trigger
  * instrument number
  * FX1 command
  * parameters for FX 1
  * FX2 command
  * parameters for FX2
- You can use the all the [standard picoTrackerkey editing combos](keypadcombos.html) for editing on the phrase screen.
- If you copy/cut anything in the phrase screen, pasting will always put the data back in the same column (regardless if you've moved the cursor to another column). This means effects in column one are always pasted back there, and you can't accidentally paste a note into the effect column, etc.
- You can clone an instrument in the phrase screen by pressing `ALT + (EDIT, ENTER)` on instrument number in phrase screen. 
- If no instrument is set when triggering a new note, tables are not stopped, running commands are not stopped and phase of oscillator instrument is not reset (allowing for clickless transitions)
- In Song mode `Play` starts and stops playback from Step 00, soloing the current phrase
- In Live mode `Play` queues the Edited Chain Step from 00
- In Song mode `Play`+`NAV` starts/stops playback of the **whole song** from where the current phrase appears in the song

## Tied Notes

Leave the instrument column empty on a new note to tie it to the previous
instrument on that track. For example:

| Note | Instrument |
|------|------------|
| C-4  | 01         |
| E-4  | --         |
| E-4  | --         |
| G-4  | 01         |

Delete the automatically filled instrument value on the rows you want to tie.
An instrument number starts a fresh note; an empty instrument column continues
the previous instrument. A note-off stops it.

### MIDI instruments

A tied note of a different pitch sends the new MIDI Note On before the previous
root note’s Note Off. MIDI ties support monophonic legato. If an `MCH` chord is
playing, its chord tones are released immediately as the tie begins. This overlap allows external synths with legato or portamento enabled
to slide between pitches. The synth determines the slide behavior and speed.
A tied note of the same pitch sends no new note messages and refreshes the
instrument's note length. `LEG` and `PSL` pitch bend continues across tied notes.
If the previous note has already stopped, the new note starts normally.

### Sample instruments

Tied notes preserve the running sample's playback position while changing pitch.
Use `LEG` or `PSL` for pitch slides (see [FX Commands Reference](commands.html)).
Sample slices continue to start at their selected slice boundaries.
