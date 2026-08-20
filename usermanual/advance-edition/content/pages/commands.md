---
title: FX Commands Reference
template: page
---

There can be upto two commands on every row of the phrase screen and upto three on a row in a table. Commands which effect instruments can be run on any step of the instruments playback, including the step where the instrument is triggered.

{% callout type=note %}
Note most commands only effect Sample or Original Sample instruments unless otherwise specified in the documentation for the specific command.
{% endcallout %}

----

## ARP abcd (ARPG in lgpt)

**cycle through relative pitches `a`, `b`, `c`, and `d` (starting with original pitch, then up `a` semitones, `b` semitones and so forth). The cycle loops if there's only zero's past a given post**

Examples:

`ARP 3000`: loops between original pitch and +3 semitones
`ARP 4050`: loops between original pitch, +4 semitones, +0 semitones, + 5 semitones

Note: speed of arpeggiator is **constant** and can not be changed

## CHN aabb

**Chance. The command targets the value immediately to its left. `bb` sets the chance amount.**

- The `aa` byte is ignored.
- In phrase command column 1, `CHN` targets the phrase step note.
- In phrase command column 2, `CHN` targets command column 1.
- In table command column 1, `CHN` targets the currently playing note.
- In table command columns 2 and 3, `CHN` targets the command column immediately to the left.
- `CHN` can also be used to gate positional commands like `HOP`.
- `bb` is a raw chance value from `00` to `FF`
  - `00` = never
  - `FF` = always
  - `80` = about 50%
- `CHN` is a control command only. It does not get sent on to the instrument itself.

Examples:

- `CHN 0080`: in phrase command column 1, the note on this row plays roughly half the time
- `VOL 0080 CHN 00C0`: the `VOL` command runs most of the time, but not always
- `HOP 0005 CHN 0080`: the `HOP` command only executes roughly half the time
- In a table, `VOL 0080 CHN 0080 --- ----`: the `VOL` in column 1 runs roughly half the time

## CSH aabb (CRSH in lgpt)

** Perform crush **

- `aa` = pre crush drive (from `1` to `0xFF`, `00` is no change) 
- `bb` = crush setting (from `0` to `0xF`, `0x0` is 1 bit, 0xF is 16bit )

## DLY --bb (DLAY in lgpt)

**Delays the note by `b + 1` ticks (only the low nibble is used).**

## FCT aabb (FCUT in lgpt)

**adjust the filter cutoff to bb at speed aa**

- `FCT 0080` will instantly set the filter cutoff to 50%
- `FCT 1000` will close the filter entirely at speed 10

## FLT aabb (FLTR in lgpt)

**lowpass filter, set absolute frequency value for cutoff aa & resonance bb**

- FLT 00FF is un-adultered sound

## FRS aabb (FRES in lgpt)

**adjust the filter resonance to bb at speed aa**

- FRS 08FF will raise the resonance to screeching at speed `08`

## REL --bb

**Releases the current instrument after `bb` ticks.**

- `REL --00` releases immediately on that step
- `REL --bb` uses the low byte as the delay, similar to `KIL --bb`
- Unlike `KIL`, `REL` sends a release to the instrument instead of hard-stopping it

## GRV aabb

**set Groove to bb**

- In Phrases, if `aa > 0` then Groove will be set for _all_ tracks
- In Tables, the Groove command has a maximum value of `1F` (31)


## HOP aabb

**play position will jump to the next phrase in a chain, jumping directly at position `bb` (low nibble) in the phrase.**

- hop is instant: instrument triggers and commands on the same row will be run.
- no effect on instruments
- `HOP` can be gated by `CHN` or randomized by `RND` commands placed to its right.
- in TABLES, cursor position will jump to row `0-F` (low nibble of `bb`) `aa` times, then pass thru the hop command and continue thru the rest of the table

## IRT --bb (IRTG in lgpt)

**Instrument Retrigger, will retrigger the current instrument. It gives a table the ability to work as progammable phrases that then can be triggered simply by changing tables.**

- IRT `--bb` will retrigger the current instrument transposed by `bb` semi-tones. Note that each IRT transposition is cumulatively added. So a table with `IRT 0001` will keep going a semitone up. Great for dubby echoes :)
- The retriggered instrument is NOT reset (as if you enter a note with no instrument number). The table (obviously) will continue to run and all running variable (filter,etc) won't be reset.
- This system is also pretty useful to implement temporary non 4/4 signature without having to switch grooves, since you have the ability to re-trigger the instrument at tick resolution
- don't forget trying to combine it with complex hop structure !

## KIL --bb (KILL in lgpt)

**instrument will stop playing after `bb` ticks.**

## LEG aabb (LEGA in lgpt)

**performs an exponential pitch slide from previous note value to pitch `bb` at speed `aa`**

- `aa` is the slide speed; the relationship is non-linear and inverted, so larger values slide more slowly and `00` is the fastest (instant)
- `bb` values are relative: 00-7F are up, 80-FF are down, expressed in semi-tones
- if `LEG` is put on a row where a note is present and the pitch offset is 0 (e.g. `C4 I3 LEG 1000`) the slide will occur automatically from the previous note's pitch to the current note's pitch at the given speed. This requires a note on the row: without one, the current note is unchanged and `bb=00` produces no slide.
- `LEG` only modulates the pitch of the voice already playing on the channel; it does not trigger or re-trigger any instrument. Whether an instrument (re)triggers is governed by the presence of an instrument number on the row, not by `LEG` (see [Tied Notes / Legato Slides](phrases.html)).
- `LEG` does exponential pitch change (i.e. it goes at the same speed through all octaves), while `PSL` performs a linear pitch slide

When used with MIDI instruments, the `LEG` command also acts as an exponential MIDI pitch bend controller:

- As with internal instruments, aa sets the speed (with 00 being instant).
- bb sets the target pitch bend position, scaled to the MIDI 14-bit pitch bend range (0–16383):
  - `7F` is the center (no bend).
  - `00` is full downward bend.
  - `FF` is full upward bend.
- MIDI pitch bend is persistent across notes — if you want to return to normal pitch, you must manually reset the bend to center.
  - This can be done by sending the aabb value `LEG 007F` on the next note or at any time.
- MIDI pitch bend can be sent without triggering a note, allowing for continuous pitch control.
 

## LOF aaaa (LPOF in lgpt)

**LooP OFset: Shift both the loop start & loop end values aaaa digits**

- `LOF 0001` adds one to both values, `LOF FFFF` removes one (so values > `0x800` moves the loop backward)
- reset everytime you start a new note (same as volume, pitch)
- `LOF` is absolute
- you can't trigger a note with the `LOF`, it has to be executed after a sample is playing
- every time you trigger a sample `LOF` is set back to the instrument parameters

## MCC aabb (MDCC in lgpt)

**Sends a MIDI “continuous control” message. aa is the control number and bb is the value. It will be sent on the MIDI channel of the currently running instrument.**

## MCH abcd

**Sends a Chord via MIDI note on messages. The notes a,b,c,d relative _semitone_ offsets from the current note as the root note of the chord.**

- For example, if the current note is C3, MCH 0047 will send a E3 note on and a G3 note on to give a C major triad chord.
- Some more examples for a C root note:
    * `0027` Suspended 2nd    **(C D G)**
    * `0036` Diminished triad **(C D# F#)**
    * `0037` Minor            **(C D# G)**
    * `0047` Major            **(C E G)**
    * `0048` Augmented        **(C E G#)**
    * `0057` Diminished 7th   **(C F G)**
    * `037A` Minor 7th        **(C D# G A)**
    * `047B` Major 7th        **(C E G B)**
- Note as the maximum of 4 notes can be sent at once, this limits the maximum chord size to a 5 note chord with the maximum distance of 15 semitones from the root note (the note on the current step).

## MPC --bb (MDPG in lgpt)

sends a program change command on the current channel. `0000` is program `0`.

## PAN aabb

**PAN aabb: where `bb` is the pan destination and aa is the speed to get there**

## PFT aabb (PFIN in lgpt)

**PitchFineTune: `aa` is the speed, and bb is a fine tune target (about +/-1 semitone).**

- `bb` sets a fractional pitch offset in roughly the range `-1 .. +1` semitone
- `00` in `bb` returns the note to the root center
- `00` is the fastest speed for `aa`

## POF aabb (PLOF in lgpt)

**PlayOFfset virtually cuts a sample into 256 chunks. It can jump absolutely to chunk `aa` and then also apply a relative signed `bb` chunk offset.**

## PSL aabb (PTCH in lgpt)

**PitchSLide performs a linear pitch slide from previous note value to pitch `bb` at speed `aa`**
- `aa` is the slide speed; the relationship is non-linear and inverted, so larger values slide more slowly and `00` is the fastest (instant)
- `bb` values are relative: `00-7F` are up, `80-FF` are down, expressed in semi-tones (same encoding as `LEG`)
- PSL performs a linear pitch change, in contrast to LEG which is exponential

The PSL command also acts as a linear MIDI pitch bend controller for MIDI instruments.

- As with internal instruments, aa sets the speed (with 00 being instant).
- bb sets the target pitch bend position, scaled to the MIDI 14-bit pitch bend range (0–16383):
  - `7F` is the center (no bend).
  - `00` is full downward bend.
  - `FF` is full upward bend.
- MIDI pitch bend is persistent across notes — if you want to return to normal pitch, you must manually reset the bend to center.
  - This can be done by sending the aabb value `PSL 007F` on the next note or at any time.
- MIDI pitch bend can be sent without triggering a note, allowing for continuous pitch control.

## RND aabb

**RaNDom. The command targets the value immediately to its left. `aabb` sets the randomization amount.**

- For note targets, `bb` sets the randomization amount and `aa` is ignored.
- In phrase command column 1, `RND` randomizes the phrase step note.
- In phrase command column 2, `RND` randomizes command column 1's parameter.
- In table command column 1, `RND` randomizes the currently playing note and retriggers the instrument.
- In table command columns 2 and 3, `RND` randomizes the parameter of the command column immediately to the left.
- For command targets, each RND field controls the matching target field. For example, `VOL 4080 RND 0010` varies only the `80` volume target, while `VOL 4080 RND 1000` varies only the `40` speed.
- Each field's value is the randomization depth:
  - `00` = no change
  - `FF` = maximum randomization
- Randomization is applied to the target command's meaningful value fields and then clamped back into the valid range for that command.
- Nibble-based commands like `ARP` and `MCH` randomize each interval nibble independently.
- Split commands like `VOL`, `FCT`, `FLT`, `FRS`, `PAN`, `PSL`, `LEG`, `PFT`, `POF`, and `RTG` randomize their two bytes independently.
- `CSH` randomizes the drive byte and the crush nibble independently.
- `MCC` keeps the controller number and randomizes only the value.
- `GRV`, `DLY`, `IRT`, `KIL`, `REL`, `TLN`, and `VEL` randomize only their active low-byte or low-nibble value.
- `HOP` randomizes both its repeat count (high byte) and destination step (low nibble) independently.
- `LOF` and `TPO` randomize as whole values.
- Control or identity commands such as `CHN`, `RND`, `STP`, `TBL`, and `MPC` are not randomized when targeted.
- Randomized note values are clamped to the valid phrase note range.

Examples:

- `RND 0080`: in phrase command column 1, randomizes the note on this row by a moderate amount
- `VOL 4080 RND 0080`: randomizes the `VOL` parameter by a moderate amount
- `HOP 0804 RND 10FF`: randomizes the `HOP` repeat count around `08` and the destination to any row from `0` to `F`
- In a table, `FCT 2080 RND 0080 --- ----`: randomizes the `FCT` parameter in column 1

## RTG aabb (RTRG in lgpt)

**Retrigger the sound by looping from the current play position over a number of ticks.**

- For Sample and Original Sample instruments: `aa` moves the loop forward by `aa` ticks each retrigger (loop offset per retrigger)
- `bb` is the loop length in ticks
- For MIDI instruments, only `bb` is used (`aa` is ignored)

`RTG 0001`: loop one tick from current play position
`RTG 0102`: loop of two ticks but move the loop one tick every loop
`RTG 0101`: does not do anything because after looping one tick, you move forward one tick and therefore go back to the current position

## TBL --bb (TABL in lgpt)

**triggers table bb**

## TLN --bb

**Sets the trigger length (gate length) of the current note to `bb` ticks.**

- For MIDI instruments, this overrides the default note length for the current step.
- For Sample and Original Sample instruments, this sets the number of ticks before the instrument is released.
- `TLN --06` results in a gate length of exactly 6 ticks (one default 16th note phrase step).
- `TLN --00` releases/stops the note immediately (gate length 0).
- Similar to `REL` or `KIL`, only the low byte `bb` is used.

## TPO aabb (TMPO in lgpt)

**sets the tempo to hex value `aabb`.**

- `TPO 0000` is clamped to the minimum tempo (`003C`, 60 BPM).
- `TPO 003C` (60bpm) is the lowest acceptable value and TPO 012C (300bpm) is the highest acceptable value.
  Values outside the allowable range will be clamped to the nearest value within the range.

## STP (STOP in lgpt)

**Stops the table from processing any further rows.**

- This command is only valid in tables, not in phrases
- When the table processor encounters STP, it immediately stops executing the current table
- Useful for creating one-shot effects that should run once and then stop

## VEL --bb

Sets the MIDI note velocity value (`bb`) for MIDI instruments. This is valid for MIDI instruments *only* and can also be used in tables.

## VOL aabb (VOLM in lgpt)

**starting from the instrument's volume setting, approach volume bb at speed aa. 00 is the lowest volume and 00 is the fastest speed (instant).**

- to achieve sounds that grow in volume, make an instrument with volume 0 and then apply the VOL command

*NOTE:* For MIDI instruments the VOL command sends MIDI CC volume (controller 7). It does **not** set note velocity.
