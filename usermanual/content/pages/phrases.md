---
title: Phrase Screen
template: page
---

![screen capture of Phrase screen](/image/phrase-screen-small.png)

- The top of the phrase screen displays the .wav file loaded into the instrument under your cursor.
- The seven columns of the phrase screen, from left to right: row counter, note trigger, instrument selector, effect one, parameters for effect one, effect two, parameters for effect two.
- You can clone instruments and tables in the phrase screen: `[NAV]+[EDIT][ENTER]` as usual. You can get a new table or instrument: `[EDIT][EDIT]`.
- If you copy/cut anything in the phrase screen, pasting will always put the data back in the same column (regardless if you've moved the cursor to another column). so effects in column one are always pasted back there, and you can't accidentally paste a note into the effect column, etc.
- you can clone a MIDI instrument in the phrase screen by pressing RT+(B,A) on instrument number in phrase screen. Properly saved & restored. You can't delete them once cloned (can't revert them to sample type).
- If no instrument is set when triggering a new note, tables are not stopped, running commands are not stopped and phase of oscillator instrument is not reset (allowing for clickless transition)