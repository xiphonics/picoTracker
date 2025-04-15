---
title: The Mixer
template: page
---

![screen capture of mixer screen](image/mixer-screen-small.png)

**NOTE: The mixer screen is only available in the version v2.1 of picoTracker firmware and later.**

The Mixer screen provides a visual overview and control center for the audio levels of each channel in your song, as well as the master output. It allows you to quickly adjust volumes, monitor audio levels and mute/solo individual channels.

## Mixer Channels

The Mixer screen displays a set of vertical stereo level (VU) meters, one for each of the eight available channels. Each channel strip represents a single channel in your song and provides the following information:

*   **Channel Level Meter:** 2 vertical bars representing the left and right channels that dynamically displays the current audio level of the channel. The higher the bar, the louder the channel's output.
*   **Peak Indicators:** The level meters also display peak indicators. These are small bars that show the highest level reached by the channel. This helps to identify if a channel is clipping.
* **Mute:** Each channel can be muted. The 'M' under each channel indicates when a channel is muted.

## Master Output

The rightmost section of the Mixer screen displays the **Master Output** level meter. This meter shows the combined audio level of all channels after they have been mixed together.

*   **Master Level Meter:** Similar to the channel level meters, the master level meter displays the overall audio level of the final mix.
*   **Peak Indicators:** The master level meter also has peak indicators to help you monitor the overall output level and avoid clipping.
* **Clip Indicator:** The master level meter has a red clip indicator at the top. If this is lit, the master output is clipping and the audio will be distorted.

## VU Meter Details

The VU meters are designed to give you a clear visual representation of the audio levels. Here's a breakdown of the meter's features:

*   **Dynamic Bars:** The bars move in real-time, reflecting the current audio level in dB.
*   **Color-Coded Regions:**
    *   **Green:** Indicates a safe and healthy audio level.
    *   **Yellow:** Indicates that the audio level is approaching the maximum.
    *   **Red:** Indicates that the audio level is clipping.
*   **Peak Hold:** The peak indicators remain visible for a short duration, allowing you to easily see the highest level reached.
* **Stereo:** Each VU meter is stereo, with the left channel on the left and the right channel on the right.

## Controls

The mixer screen is primarily a monitoring tool, but it also provides same channel control key combos as  are available on the song screen:

* `NAV`+`EDIT`: Toggles mute/unmute of cursor channel
    * if `NAV` is released before `EDIT`, channel stays mutes
    * if `EDIT` is released before `NAV`, channel goes back to original state
* `NAV`+`ENTER`: Solo cursor channel
    * if `NAV` is released before `ENTER`, channel stays solo'ed
    * if `ENTER` is released before `NAV`, all channel go back to original state
* `ALT`+`NAV`: restore full playback on all channels
* `NAV`+`ENTER`,`NAV`+`EDIT` can be used in conjunction with selections. 
    * if a selection is present the toggle mute/solo action is done on all channels present in the selection


