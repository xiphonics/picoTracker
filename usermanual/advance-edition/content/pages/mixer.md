---
title: The Mixer
template: page
---

![screen capture of mixer screen](image/mixer-screen-small.png)

The Mixer screen provides a visual overview and control center for the audio levels of each channel in your song, as well as the master output. It allows you to quickly adjust volumes, monitor audio levels, control output-effect sends and mute/solo individual channels.

## Mixer Channels

The Mixer screen displays a set of vertical stereo level (VU) meters, one for each of the eight available channels. Each channel strip represents a single stereo channel in your song and provides the following information:

*   **Channel Level Meter:** 2 vertical bars representing the left and right channels that dynamically displays the current audio level of the channel. The higher the bar, the louder the channel's output.
* **Mute:** Each channel can be muted. The 'M' under each channel indicates when a channel is muted.
* **FX1–FX4:** Each channel has four post-fader send levels. A send routes the channel to the corresponding output-effect slot without removing it from the dry master mix.

The effect names at the left of the FX rows are selectable. Move left from channel 1 on an FX row, then hold <span class="minikeys">ENTER</span> and press <span class="minikeys">UP</span> or <span class="minikeys">DOWN</span> to select the effect for that slot.

Effect types cannot be changed while the player is running. Changing a type while stopped performs a kill first: sounding notes and any remaining effect tails stop immediately.

## Master Output

The rightmost section of the Mixer screen displays the **Master Output** level meter. This meter shows the combined audio level of all channels after they have been mixed together.

*   **Master Level Meter:** Similar to the channel level meters, the master level meter displays the overall audio level of the final mix.
* **Clip Indicator:** The master level meter has a red clip indicator at the top. If this is lit, the master output is clipping and the audio will be distorted.

## Master Effects

Press <span class="minikeys">NAV</span>+<span class="minikeys">DOWN</span> from the Mixer to open **Master FX**. Output effects reserve one to four resource units. Each unit reserves part of the processing power and working space available to the effects. The four slots share four units. The slot's **SLOT RES** value shows its reservation, and the title shows the total in use. Effect choices that would exceed the budget are unavailable. Selecting `NONE` always releases the slot's reservation.

`RING` selects Rings reverb and reserves two units. `FREE` selects Freeverb and reserves three units. Both provide the same input, time, diffusion and low-pass controls.

The displayed slot provides:

* **RETURN:** The level of that slot's wet output mixed into the master output.
* **Effect parameters:** Controls belonging to the selected effect.
* **TO2–TO4:** Sends from the selected slot to each later slot. As with channel sends, `00` is off and `99` sends the slot's fully wet processed output at full level.

Slot routing is forward-only: FX1 can feed FX2–FX4, FX2 can feed FX3–FX4, and FX3 can feed FX4. These wet-output sends are applied before `RETURN`, so a slot can feed a later effect while its own master return is zero. All slot-to-slot sends default to zero, preserving a parallel-effects layout until routing is added.

## VU Meter Details

The VU meters are designed to give you a clear visual representation of the audio levels. Here's a breakdown of the meter's features:

*   **Dynamic Bars:** The bars move in real-time, reflecting the current audio level in dB.
*   **Color-Coded Regions:**
    *   **Green:** Indicates a safe and healthy audio level.
    *   **Yellow:** Indicates that the audio level is approaching the maximum.
    *   **Red:** Indicates that the audio level is clipping.
* **Stereo:** Each VU meter is stereo, with the left channel on the left and the right channel on the right.

## Controls

The mixer screen is primarily a monitoring tool, but it also provides same channel control key combos as  are available on the song screen:

* <span class="minikeys">NAV</span>+<span class="minikeys">EDIT</span>: Toggles mute/unmute of cursor channel
    * if <span class="minikeys">NAV</span> is released before <span class="minikeys">EDIT</span>, channel stays mutes
    * if <span class="minikeys">EDIT</span> is released before <span class="minikeys">NAV</span>, channel goes back to original state
* <span class="minikeys">NAV</span>+<span class="minikeys">ENTER</span>: Solo cursor channel
    * if <span class="minikeys">NAV</span> is released before <span class="minikeys">ENTER</span>, channel stays solo'ed
    * if <span class="minikeys">ENTER</span> is released before <span class="minikeys">NAV</span>, all channel go back to original state
* <span class="minikeys">ALT</span>+<span class="minikeys">NAV</span>: restore full playback on all channels
