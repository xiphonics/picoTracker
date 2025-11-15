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
- You can clone an instrument in the phrase screen by pressing `NAV`+`ENTER`,`EDIT` on instrument number in phrase screen. 
- If no instrument is set when triggering a new note, tables are not stopped, running commands are not stopped and phase of oscillator instrument is not reset (allowing for clickless transitions)
- In Song mode `Play` starts and stops playback from Step 00, soloing the current phrase
- In Live mode `Play` queues the Edited Chain Step from 00
- In Song mode `Play`+`NAV` starts/stops playback of the **whole song** from where the current phrase appears in the song