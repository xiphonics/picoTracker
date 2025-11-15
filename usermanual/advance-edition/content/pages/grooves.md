---
title: Grooves
template: page
---

![screen capture of groove screen](image/groove-screen-small.png)

## Introduction to Grooves

The Groove screen allows you to modify the timing of steps in your patterns, creating swing, shuffle, and other rhythmic variations. Grooves are an essential tool for adding a more human feel to your compositions and breaking away from rigid timing.

## Understanding Ticks

To understand how grooves work, it's important to first understand the concept of ticks:

- A tick is the smallest unit of time measurement in picoTracker
- By default, each row (step) in a phrase receives 6 ticks
- The groove screen allows you to redistribute these ticks across steps

## How Grooves Work

The groove screen displays a sequence of numbers, each representing the number of ticks assigned to consecutive steps in your phrases:

- The default groove is "6/6" - meaning each step gets 6 ticks (equal timing)
- If you change this to "9/3", odd-numbered steps will last 9 ticks while even-numbered steps will last 3 ticks
- This creates a swing or shuffle feel, as every other step is played longer

## Groove Examples

| Groove Pattern | Effect |
|----------------|--------|
| 6/6 | Default timing, equal distribution |
| 9/3 | Classic swing feel with longer first beat |
| 4/8/4 | Three-step pattern with emphasis on middle beat |
| 1/F | Extreme swing with very short first beat (1) and very long second beat (15) |

## Experimenting with Grooves

To understand how grooves affect playback:

1. Set a groove of "1/F" and watch the play cursor in the phrase screen
2. Notice how the cursor stays on even-numbered steps much longer than odd-numbered steps
3. Try a groove of "1/1/F" and observe how the cursor now emphasizes every third step
4. Experiment with more subtle values for different rhythmic feels

## Groove Screen Controls

The following controls are available in the groove screen:

- `EDIT`: Add a new step to the groove pattern if one doesn't exist
- `EDIT`+`Left/Right`: Modify the current step value
- `ENTER`+`Arrows`: Navigate between different groove patterns
- `EDIT`+`ENTER`: Clear the current step

## Tips for Using Grooves

- Subtle groove settings (like 7/5) can add a slight human feel without being obvious
- More extreme settings can create distinctive rhythmic patterns
- Grooves can be triggered from tables using the GRV command
- Different groove patterns can be used in different sections of your song

The GRV command (which is only active in the phrase screen) selects the current groove.