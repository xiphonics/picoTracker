---
title: Song Screen
template: page
---

![screen capture of song screen](image/song-screen-small.png)

Songs are made up of chains. Each hex number on the song screen grid is a chain. You can navigate to any of those chains by putting your cursor on a chain number and hitting <span class="minikeys">NAV</span>+<span class="minikeys">RIGHT</span>.

### Screen elements

* The top of the song screen displays if you are in SONG or LIVE mode (toggle with <span class="minikeys">EDIT</span>+<span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span>) and the name of the project that is currently open.

* When a song channel is assigned to MIDI input, `IN` row appears above the song grid and shows its incoming MIDI channel. Unassigned columns stay blank. MIDI input assignments are edited on the Project screen.

* At the bottom of the screen are the channel playback visualiser boxes. Each box correlates with the above column in the song grid. When a step is triggered in one of the columns, the note value of that trigger is displayed in the play-time visualizer.

* On the top right of the song screen we have play-time statistics. First there is the clipping indicator which displays “----” when your volume levels are ok and “clip” when you've cleared the headroom and are chopping off the top of your samples. The bottom row of the play-time statistics is the amount of time which has passed since you pressed Play.

### Controls

* You can toggle between "SONG" or "LIVE" mode with <span class="minikeys">EDIT</span>+<span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span>

* You can navigate through the grid of chains using <span class="minikeys">UP</span>/<span class="minikeys">DOWN</span>/<span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span> 

* An assigned song channel is reserved for MIDI input and its song data is shown dimmed. That data is kept in the project and becomes playable again when the assignment is cleared on the Project screen. Assigning the same MIDI channel to more than one song channel allows that MIDI channel to play multiple notes at once.

* You can jump the cursor to the next/previous chain in a column by pressing <span class="minikeys">NAV</span>+<span class="minikeys">DOWN</span>/<span class="minikeys">UP</span>

* See the [Controls & Moves reference](keypadcombos.html) for a list of all the controls you can use for editing the chains grid on the song screen.

### Selection Resampling

You can resample a selection directly from the Song screen.

* Start a grid selection first.
* Press <span class="minikeys">ENTER</span>+<span class="minikeys">PLAY</span> while the selection is active.
* picoTracker renders only the selected song rows and selected channels.
* Playback must be stopped before starting the resample.
* The rendered audio is written to the current project's samples folder as `resample.wav` and then loaded into the project sample pool.
* If the selected area contains no playable content, resampling will not start.

### Solo and Mute

You can solo and mute specific channels on the song screen using the following key combos:

* <span class="minikeys">NAV</span>+<span class="minikeys">EDIT</span>: Toggles mute/unmute of cursor channel
    * if <span class="minikeys">NAV</span> is released before <span class="minikeys">EDIT</span>, channel stays mutes
    * if <span class="minikeys">EDIT</span> is released before <span class="minikeys">NAV</span>, channel goes back to original state
* <span class="minikeys">NAV</span>+<span class="minikeys">ENTER</span>: Solo cursor channel
    * if <span class="minikeys">NAV</span> is released before <span class="minikeys">ENTER</span>, channel stays solo'ed
    * if <span class="minikeys">ENTER</span> is released before <span class="minikeys">NAV</span>, all channel go back to original state
* <span class="minikeys">ALT</span>+<span class="minikeys">NAV</span>: restore full playback on all channels
* <span class="minikeys">NAV</span>+<span class="minikeys">ENTER</span>,<span class="minikeys">NAV</span>+<span class="minikeys">EDIT</span> can be used in conjunction with selections. 
    * if a selection is present the toggle mute/solo action is done on all channels present in the selection

### Playback Modes

As mentioned above, the Song screen can be either in *song* or *live* mode. The controls in each mode differ slightly. You can switch between the modes using <span class="minikeys">ENTER</span>+<span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span> while on the Song screen. 

### Tempo Nudge

You can temporarily speed up or slow down the song playback using the Tempo Nudge feature. This is useful for syncing with external gear or making fine timing adjustments during live/DJ performances.

- <span class="minikeys">ALT</span> + <span class="minikeys">LEFT</span>: Nudge tempo down (slower)
- <span class="minikeys">ALT</span> + <span class="minikeys">RIGHT</span>: Nudge tempo up (faster)

The tempo will return to its original value when you release the keys.

#### Song Mode

In the song mode <span class="minikeys">Play</span> starts and stops song playback from the currently highlighted row of the chain grid. If one of the rows channel is marked `--`, that channel will be ignored entirely for the rest of the playback session (until playback is stopped). 

Hold <span class="minikeys">PLAY</span> for about one second to panic stop. This immediately stops playback and cuts currently sounding audio, including stuck notes or long effect tails. A normal short press of <span class="minikeys">PLAY</span> still starts or stops playback when you release the key.

In Song mode <span class="minikeys">ALT</span>+<span class="minikeys">PLAY</span> works the same as <span class="minikeys">PLAY</span>, starting or stopping song playback from any screen.

#### Live Mode

In Live mode <span class="minikeys">Play</span> queues from the currently highlighted channel step.
* The Queued item will be played as soon as the playing chain on the selected channel reaches its last step. 
* If there is no playing chain step on the selected channel, the next song chain that reaches its last step will trigger playback. 
* Queued chains are shown with a blinking `>`.
* Pressing <span class="minikeys">Play</span> a second time will queue the chain using "immediate" mode. 
* The queued item will be played as soon as the playing phrase on its channel reaches the last step. 
* Immediate mode Queued items are shown with a fast blinking `>`.

<span class="minikeys">ALT</span>+<span class="minikeys">PLAY</span> will queue all channel steps on the current row.
* The queued items will be played as soon as the playing chain on their channel reaches its last step. 
* Queued items are shown with a blinking `>`. 
* Pressing <span class="minikeys">Play</span> a second time will queue the items using immediate mode. 
* The queued items will be played as soon as the playing Phrase on the their xhannel reaches its last step.
* Immediate mode queued items are shown with a fast blinking `>`.

<span class="minikeys">NAV</span>+<span class="minikeys">Play</span>: Queues the selected channel step to be stopped. 
* The queued channel will be stopped as soon as its playing chain reaches the last step.
* Queued Items are shown with a blinking `_`. 
* Pressing <span class="minikeys">Play</span> a second time will cause the selected channel step to be queued to stop using immediate mode.
* The queued channel will be stopped as soon as its playing phrase reaches the last step. 
* Immediate mode queued items are shown with a fast blinking `_`.
