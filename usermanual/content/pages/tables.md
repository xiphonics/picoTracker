---
title: Tables
template: page
---

# Tables

![screen capture of table screen](image/table-screen-small.png)

## Introduction to Tables

Tables are one of the most powerful features in picoTracker, allowing you to create complex effect sequences, parameter automation, and even custom arpeggiators. Tables work as a sequence of commands that can be triggered from phrases or directly from instruments.

## Table Basics

- picoTracker provides 16 tables (0-15) that can be used throughout your project
- Each table can contain up to 16 rows of commands
- Tables can be looped, played once, or used to jump between different sections
- You can navigate between tables by pressing `EDIT`+`LEFT`/`RIGHT`

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

You can use all the [standard picoTracker key editing combos](keypadcombos.html) for navigating and editing the table screen.

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
03 JMP 0000 --- --- --- ---
```

This creates a simple major arpeggio pattern.

### Parameter Automation

Tables excel at automating parameters over time:

```
00 VOL FF PAN 00 --- ---
01 VOL F0 PAN 10 --- ---
02 VOL E0 PAN 20 --- ---
...
07 VOL 80 PAN 80 --- ---
```

This creates a fade-in effect while panning from left to right.

### Complex Effect Chains

Combine multiple effects for rich sound design:

```
00 VOL FF FRQ 0100 --- ---
01 VOL F0 FRQ 0101 --- ---
02 VOL E0 FRQ 0102 --- ---
03 VOL D0 FRQ 0103 --- ---
04 JMP 0000 --- --- --- ---
```

This creates a pulsing effect with rising pitch.

## Tips and Tricks

- Tables are processed at the same rate as the phrase's tick rate
- Use the HOP command to create repeating patterns within a table
- Combine multiple tables using JMP for complex sequences
- For subtle effects, use small parameter changes between rows
- Tables can be shared between multiple instruments
- After Hopping count is reached, the table pointer moves directly to the line after the HOP rather than staying on the HOP line for one tick
