---
title: Phrase Screen
template: page
---

![screen capture of Phrase screen](image/phrase-screen-small.png)

- The top of the phrase screen displays the name of the instrument under your cursor.
- The seven columns of the phrase screen, from left to right are: 
  * row counter
  * note trigger| instrument selector
  * FX1 command
  * parameters for FX 1
  * FX2 command
  * parameters for FX2
- You can clone instruments and tables in the phrase screen with: `NAV`+`EDIT ENTER`. You can create a new table or instrument with: `EDIT EDIT`.
- If you copy/cut anything in the phrase screen, pasting will always put the data back in the same column (regardless if you've moved the cursor to another column). This means effects in column one are always pasted back there, and you can't accidentally paste a note into the effect column, etc.
- You can clone a MIDI or other type of instrument in the phrase screen by pressing RT+(B,A) on instrument number in phrase screen. 
- If no instrument is set when triggering a new note, tables are not stopped, running commands are not stopped and phase of oscillator instrument is not reset (allowing for clickless transitions)