---
title: Tables
template: page
---

![screen capture of table screen](image/table-screen-small.png)

## Introduction to Tables

Tables are one of the most powerful features in picoTracker, allowing you to create complex effect sequences, parameter automation, and even custom arpeggiators. Tables work as a sequence of commands that can be triggered from phrases or directly from instruments.

## Table Basics

- picoTracker provides 16 tables (0-15) that can be used throughout your project
- Each table can contain up to 16 rows of commands
- Tables can be looped, played once, or used to jump between different sections
- You can navigate between tables by pressing `EDIT`+`LEFT`/`RIGHT`
- You can use all the [standard picoTracker key editing combos](keypadcombos.html) for navigating and editing the table screen.


## Table Interface

The table screen consists of seven columns:

| Column | Description |
|--------|-------------|
| Row # | The row number (00-15) |
| FX1 | First effect command |
| Param1 | Parameter for the first effect |
| FX2 | Second effect command |
| Param2 | Parameter for the second effect |
| FX3 | Third effect command |
| Param3 | Parameter for the third effect |


## Using Tables from Phrases

Tables can be triggered directly from the phrase screen by using the `TBL` command in the effect column:

```
C-4 01 -- TBL 03
```

This will trigger table 3 when this step in the phrase is played. The table will be applied to the instrument in the current track.

When a table is triggered from a phrase:

1. The table starts playing from row 0
2. Each row in the table is processed at the same rate as the phrase's tick rate
3. The table effects are applied to the currently playing note

## Assigning Tables to Instruments

Tables can also be assigned directly to sampler instruments:

1. Navigate to the instrument edit screen
2. Find the `table:` field at the bottom of the screen
3. Set it to the table number you want to assign (0-15)
4. Use the `automation` field to control hwo the table is triggered

When a table is assigned to an instrument, it will automatically trigger whenever that instrument plays a note.

## Automation Settings

The `automation` field controls how the table state is handled when an instrument plays a note:

| Setting | Behavior |
|---------|----------|
| OFF | Table state is reset each time the instrument plays |
| ON | Table state is saved with the instrument between notes |

When automation is ON, the table's current position and state are saved with the instrument. This means that if you play a note with that instrument, then play another note later, the table will continue from where it left off rather than starting over from the beginning.

This is particularly useful for creating evolving sounds or complex effects that span across multiple notes. For example, you could create a table that slowly changes a filter cutoff or panning position across several notes, creating a continuous effect even when playing different notes with pauses between them.

When automation is OFF, the table will always start from the beginning each time the instrument plays a new note, which is useful for effects that should be consistent for each note.

## Table Commands

Here are some commonly used table commands:

### HOP - Jump to Another Row

```
HOP 0305
```

Jumps to row 03 of the current table and repeats this 05 times before continuing.

Hopping to the same row creates a hold effect. For example:

```
05 HOP 0505
```

This will hold at row 05 for 5 ticks before continuing.

Hopping to self is particularly useful for creating complex envelopes. For example:

```
00 VOL 0400 ; starts short volume decay to zero
01 HOP 0110 ; holds enough ticks for VOL to complete
02 VOL 0560 ; raise volume to 60
03 HOP 0303 ; hold for a long time to allow VOL to complete and hold volume at 60
```

### STOP - Stop Table Execution

```
STOP
```

Stops the table from processing any further rows.

### VOL - Set Volume

```
VOL 40
```

Sets the volume to 64 (40 in hex).

### PAN - Set Panning

```
PAN 80
```

Sets the panning to center (80 in hex, range 00-FF).

### FRQ - Frequency/Pitch Adjustment

```
FRQ 0108
```

Adjusts the pitch up by 8 semitones (08) in the first oscillator (01).

## Creative Uses for Tables

### Creating Arpeggiators

You can create custom arpeggiators by using the FRQ command to change the pitch in a sequence:

```
00 FRQ 0100 --- --- --- ---
01 FRQ 0104 --- --- --- ---
02 FRQ 0107 --- --- --- ---
03 HOP 0003 --- --- --- ---
04 HOP 0000 --- --- --- ---
```

This creates a simple major arpeggio pattern that repeats 3 times before looping back to the beginning.

### Trance Gate Effect

Create a rhythmic gating effect by rapidly changing volume:

```
00 VOL FF --- --- --- ---
01 VOL 00 --- --- --- ---
02 VOL FF --- --- --- ---
03 VOL 00 --- --- --- ---
04 VOL 80 --- --- --- ---
05 VOL 00 --- --- --- ---
06 VOL FF --- --- --- ---
07 HOP 0000 --- --- --- ---
```

This creates a rhythmic on/off pattern that's great for dance music.

### Vibrato Effect

Create a vibrato effect by oscillating pitch slightly:

```
00 FRQ 0101 --- --- --- ---
01 FRQ 0100 --- --- --- ---
02 FRQ 00FF --- --- --- ---
03 FRQ 0100 --- --- --- ---
04 HOP 0000 --- --- --- ---
```

This creates a subtle pitch wobble that adds character to sustained notes.

### Parameter Automation

Tables excel at automating parameters over time:

```
00 VOL FF PAN 00 --- ---
01 VOL F0 PAN 10 --- ---
02 VOL E0 PAN 20 --- ---
03 VOL D0 PAN 30 --- ---
04 VOL C0 PAN 40 --- ---
05 VOL B0 PAN 50 --- ---
06 VOL A0 PAN 60 --- ---
07 VOL 90 PAN 70 --- ---
08 VOL 80 PAN 80 --- ---
09 STOP -- --- --- --- ---
```

This creates a fade-in effect while panning from left to right, then stops.

### Filter Sweep

Create a filter sweep effect:

```
00 FCUT 00 --- --- --- ---
01 FCUT 20 --- --- --- ---
02 FCUT 40 --- --- --- ---
03 FCUT 60 --- --- --- ---
04 FCUT 80 --- --- --- ---
05 FCUT A0 --- --- --- ---
06 FCUT C0 --- --- --- ---
07 FCUT E0 --- --- --- ---
08 FCUT FF --- --- --- ---
09 HOP 0000 --- --- --- ---
```

This gradually opens a filter from fully closed to fully open.

### Complex Drum Pattern

Use a table to create a complex drum pattern with retriggers:

```
00 RTG 04 --- --- --- ---
01 VOL 80 --- --- --- ---
02 VOL 40 --- --- --- ---
03 VOL 20 --- --- --- ---
04 HOP 0703 --- --- --- ---
05 RTG 08 --- --- --- ---
06 VOL FF --- --- --- ---
07 HOP 0000 --- --- --- ---
```

This creates a drum pattern with initial quick retriggers, then a pause, followed by faster retriggers.

### Sample Position Effects with POF

Create interesting effects by manipulating the playback position of samples using POF (Play Offset):

```
00 VOL FF --- --- --- ---
01 POF 1000 --- --- --- ---
02 VOL C0 --- --- --- ---
03 POF 2000 --- --- --- ---
04 VOL 80 --- --- --- ---
05 POF 3000 --- --- --- ---
06 VOL 40 --- --- --- ---
07 STP -- --- --- --- ---
```

This jumps to different positions in the sample while decreasing the volume, creating a stuttering effect that can sound like echoes or glitches depending on the sample content. The POF command takes a hexadecimal offset value that determines where in the sample to start playback from.

### Multi-Parameter Modulation

Combine multiple effects for rich sound design:

```
00 VOL FF FCT 20 PAN 40
01 VOL E0 FCT 40 PAN 50
02 VOL C0 FCT 60 PAN 60
03 VOL A0 FCT 80 PAN 70
04 VOL 80 FCT A0 PAN 80
05 VOL A0 FCT 80 PAN 70
06 VOL C0 FCT 60 PAN 60
07 VOL E0 FCT 40 PAN 50
08 HOP 0000 --- --- --- ---
```

This creates a complex pattern that simultaneously modulates volume, filter cutoff, and panning in a wave-like pattern.

## Tips and Tricks

- Tables are processed at the same rate as the phrase's tick rate
- Use the HOP command to create repeating patterns within a table
- Combine multiple tables using JMP for complex sequences
- For subtle effects, use small parameter changes between rows
- Tables can be shared between multiple instruments
- After Hopping count is reached, the table pointer moves directly to the line after the HOP rather than staying on the HOP line for one tick
