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
* **DLY and RVB:** Each channel has post-fader sends for delay and reverb. Under the master channel column is the level of the effects returns.

The effect names at the left of the rows are fixed. Press <span class="minikeys">NAV</span>+<span class="minikeys">DOWN</span> on an effect row to open its Master Effects page.

## Master Output

The rightmost section of the Mixer screen displays the **Master Output** level meter. This meter shows the combined audio level of all channels after they have been mixed together.

*   **Master Level Meter:** Similar to the channel level meters, the master level meter displays the overall audio level of the final mix.
* **Clip Indicator:** The master level meter has a red clip indicator at the top. If this is lit, the master output is clipping and the audio will be distorted.

## Master Effects

Press <span class="minikeys">NAV</span>+<span class="minikeys">DOWN</span> from the Mixer to open **Master Effects**.

`DLY` is a stereo delay. Its controls are:

* **Time:** Sets the delay time in audio ticks so echoes align with tracker timing. A 16-step bar contains 96 ticks: `01` is one tick, `06` is one step, `12` is two steps, `24` is four steps, `48` is half a bar, and `96` is the maximum of one full bar. The delay follows the song tempo.
* **Feedback:** Sets how much of each echo returns to the delay. `00` produces one echo with no repetitions. `FF` gives the longest decay but remains just below a sustained loop.

`RVB` is a stereo reverb. Its controls are:

* **Pre-delay:** Sets the time between the original sound and the beginning of the reverb. `00` applies no pre-delay and `FF` applies the longest pre-delay.
* **Input tone:** Sets the brightness of sound entering the reverb. `00` is darkest and `FF` is brightest.
* **Diffuse:** Sets how much the sound if diffused.
* **Decay time:** Sets how long the reverb takes to fade. `00` is the shortest finite decay, `FE` is the longest finite decay, and `FF` holds the reverb without fading.
* **Tail tone:** Sets the brightness of the fading reverb. `00` produces the darkest tail and `FF` the brightest.
* **Mod speed:** Sets the speed of movement in the reverb. `00` stops the movement and `FF` is the fastest rate.
* **Mod level:** Sets how strongly that movement affects the reverb. `00` turns it off and `FF` gives the strongest movement.

`RVB` control changes take effect immediately while playing. The effect output is always fully wet: channel sends control how much signal enters the reverb, and the effect return in the Mixer's rightmost column controls how much reverb is mixed into the master output.

The same controls can be automated from phrases or tables with the `RVP`, `RVI`,
`RVF`, `RVD`, `RVT`, `RVS`, and `RVL` commands.

The delay column also provides **RVB send**, which sends the delay's wet output into the reverb.

The delay-to-reverb send is applied before the delay **RETURN**, so the delay can feed the reverb while its own master return is zero. It defaults to zero, preserving a parallel-effects layout until routing is added.

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
