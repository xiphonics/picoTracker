---
title: Record Screen
template: page
---

![record screen screenshot](image/record-screen-small.png)

## Introduction

The Record screen allows you to capture audio from external sources directly into the picoTracker. You can record from a line-level source or the built-in microphone, making it easy to sample sounds from your environment or other gear.

## Accessing the Record Screen

You can access it from any screen by pressing `EDIT` + `PLAY` when the sequencer is stopped.

## Screen Layout

The Record screen displays the following information:

*   **Recording Status:** `[REC]` is shown in red when recording is active, and `[---]` is shown otherwise.
*   **Recording Timer:** A timer displays the duration of the current recording in `MM:SS` format.
*   **VU Meter:** A VU meter on the right of the screen shows the input audio level.

## Recording Controls

*   **PLAY:** Press the `PLAY` button to start recording. If a recording is in progress, pressing `PLAY` will stop it.
*   **NAV + LEFT:** Press `NAV` + `LEFT` to exit the Record screen and return to the Song view. If a recording is in progress, it will be discarded.

## Recording Settings

You can configure the following settings on the Record screen:

*   **Audio source:** Choose between `Line In` and `Mic`.
*   **Line gain:** Adjust the input gain for the line-in source.
*   **Mic gain:** Adjust the input gain for the microphone.

Use the `ARROW` keys to navigate between these settings and `EDIT` + `LEFT`/`RIGHT` to change their values.

## The Recording Process

1.  Navigate to the Record screen.
2.  Select your desired audio source and adjust the gain.
3.  Press `PLAY` to start recording. The status indicator will turn to `[REC]`, and the timer will start.
4.  When you are finished, press `PLAY` again to stop.
5.  The picoTracker will save the recording as `REC01.wav` in the `/recordings` directory on your SD card.
6.  After saving, the picoTracker will automatically switch to the Sample Editor screen with the new recording loaded, ready for you to edit and use.

## Tips

*   Keep an eye on the VU meter to avoid clipping (when the meter hits the red) and also to have a high enough signal. Adjust the gain accordingly for a clean recording.
*   The recorded sample will replace any existing `REC01.wav` file in the `/recordings` directory, so be sure to rename your recordings in the Sample Editor if you want to keep them.
