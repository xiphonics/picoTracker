---
title: FX Commands Reference
template: page
---

There can be upto two commands on every row of the phrase screen and upto three on a row in a table. Commands which effect instruments can be run on any step of the instruments playback, including the step where the instrument is triggered.
in vol; pitch and kill but the definition of the “time' is slightly different for all command…

## ARP abcd (ARPG in lgpt)

**cycle through relative pitches a, b, c, and d (starting with original pitch, then up a semitones, b semitones and so forth). The cycle loops if there's only zero's past a given post**
Examples:
ARP 3000: loops between original pitch and +3 semitones
ARP 4050: loops between original pitch, +4 semitones, +0 semitones, + 5 semitones

- speed of arpeggiator is constant and can not be changed
## CSH aabb (CRSH in lgpt)

**aa = pre crush drive (from 1 to 0xFF, 00 is no change) & bb = crush setting (from 0 to 0xF, 0x0 is 1 bit, 0xF is 16bit )**

## DLY --bb (DLAY in lgpt)

**Delays the note to be played by bb tics**

## FCT aabb (FCUT in lgpt)

**adjust the filter cutoff to bb at speed aa**

- FCT 0080 will instantly set the filter cutoff to 50%
- FCT 1000 will close the filter entirely at speed 10
## FLT aabb (FLTR in lgpt)

**lowpass filter, set absolute frequency value for cutoff aa & resonance bb**

- FLT 00FF is un-adultered sound
## FRS aabb (FRES in lgpt)

**adjust the filter resonance to bb at speed aa**

- FRS 08FF will raise the resonance to screeching at speed 08

## HOP aabb

**play position will jump to the next phrase in a chain, jumping directly at position bb in the phrase.**

- hop is instant: instrument triggers and commands on the same row will be run.
- no effect on instruments
- in TABLES, cursor position will jump to row bb aa times, then pass thru the hop command and continue thru the rest of the table

## IRT aabb (IRTG in lgpt)

IRT stands for Instrument Retrigger and will retrigger the current instrument. It gives table the ability to work as progammable phrases that then can be triggered simply by changing tables.
IRT –bb will retrigger the current instrument transposed by bb semi-tones. Note that each IRT transposition is cumulatively added. So a table with
IRT 0001
will keep going a semi tone up. Great for dubby echoes :)
The retriggered instrument is NOT reset (as if you enter a note with no instrument number). The table (obviously) will continue to run and all running variable (filter,etc) won't be reset.
This system is also pretty useful to implement temporary non 4/4 signature without having to switch grooves, since you have the ability to re-trigger the instrument at tick resolution
don't forget trying to combine it with complex hop structure !

## KIL --bb (KILL in lgpt)

**instrument will stop playing after aa ticks.**

## LEG aabb (LEGA in lgpt)

**performs an exponential pitch slide from previous note value to pitch bb at speed aa.**

- 00 is the fastest speed for aa (instant, useless)
- bb values are relative: 00-7F are up, 80-FF are down, expressed in semi-tones
- if LEG is put on a row where a note is present and the pitch offset is 0 (e.g. `C4 I3 LEG 1000`) the slide will occur automatically from previous note to the current one at the given speed.
- If an instrument is not triggered on the same row as LEG, the command will re-trigger the previous instrument (unless the previous instrument is still playing).
- LEG does exponential pitch change (i;e. it goes at same speed through all octaves) while PITCH is linear

## LOF aaaa (LPOF in lgpt)

**LooP OFset: Shift both the loop start & loop end values aaaa digits**

- LOF 0001 adds one to both values, LOF FFFF removes one (so values > 0x800 moves the loop backward)
- reset everytime you start a new note (same as volume, pitch)
- LOF is absolute
- you can't trigger a note with the LOF, it has to be executed after a sample is playing
- every time you trigger a sample LOF is set back to the instrument parameters
## MCC aabb (MDCC in lgpt)

**Sends a MIDI “continuous control” message. aa is the control number and bb is the value. It will be sent on the MIDI channel of the currently running instrument.**

## MPC --bb (MDPG in lgpt)

sends a program change command on the current channel. 0000 is program change 1

## PAN aabb

**PAN aabb: where bb is the pan destination and aa is the speed to get there**

## PFT aabb (PFIN in lgpt)

**PitchFineTune: where bb is the width and aa is the speed to get there**

- Tunes the root note one semitone up (01-80) or down (FF-81)
- 00 in bb returns the note to the root center
- 00 is the fastest speed for aa

## POF aabb (PLOF in lgpt)

**PlayOFfset virtually cuts any sample in 256 chunks. jump absolutely to chunk aa or relatively move forward/back bb chunks.**

## PSL aabb (PTCH in lgpt)

**PitchSLide performs a linear pitch slide from previous note value to pitch bb at speed aa**
- PSL is also time for the first two byte nibble
- PITCH is linear pitch change

## RTG aabb (RTRG in lgpt)

**retrigger the sound by looping the from current play position over a certain amount of ticks.**

- aa allows to move the loop forward of aa ticks each time the loop has been done (loop offest per retrigger)
- bb is the number of ticks used for the looping (speed of retrigger effect)

RTG 0001: loop one tick from current play position
RTG 0102: loop of two ticks but move the loop one tick every loop
RTG 0101: does not do anything because after looping one tick, you move forward one tick and therefore go back to the current position :)

## TBL --bb (TABL in lgpt)

**triggers table bb**

## TPO --bb (TMPO in lgpt)

**sets the tempo to hex value –bb.**

- TPO 0000 is safe and doesn't effect the tempo at all.
- TPO 003C (60bpm) is the lowest acceptable value and TPO 0190 (400bpm) is the highest acceptable value

## VEL --bb

Set the velocity of the note being played on the current step for a MIDI instrument. This valid for MIDI instruments *only* and this command is not supported for use in tables.

## VOL aabb (VOLM in lgpt)

**starting from the instrument's volume setting, approach volume bb at speed aa. 00 is the lowest volume and 00 is the fastest speed (instant).**

- to achieve sounds that grow in volume, make an instrument with volume 0 and then apply the VOL command

*NOTE:* For MIDI instruments the VOL command sets the velocity for that step. Only 1 VOL command in the first FX column is supported for MIDI. Only bb is used for velocity, aa has no effect for MIDI instruments.