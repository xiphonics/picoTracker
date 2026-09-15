---
title: Project Management
template: page
---

![screen capture of project screen](image/project-screen-small.png)

On the project screen you can change various settings for the current project, save it, rename it (including giving it a random name), create a new blank project, or browse for another project to load. You can also render your song to audio files.

Your current project settings are saved automatically every minute except when the sequencer is running (i.e., when the current project is playing). This means that should you restart the picoTracker, accidentally power off, or if a crash occurs, your project state from within the last minute will be restored.

You can ***explicitly*** save the current project by selecting **Save**. This is recommended before performing major changes or after a successful session.

## Current Project Settings

- **tempo:** Can be set between 60bpm and 400bpm. You can also tap tempo: move the cursor to the `tempo` field and press **EDIT** repeatedly in time.
- **master vol:** Sets the overall master output volume from 0% to 100%.
- **transpose:** Live transposition of every triggered instrument in semitones (-48 to +48).
- **scale:** Set the scale that will be applied to all notes entered in the project. When entering a note in the Phrase screen, you will only be able to enter notes that belong to the selected scale. See [the reference](scales.html) for a list of all available scales.
- **scale root:** Sets the root note for the selected scale (e.g., C, C#, D, etc.).

For best tap tempo results, tap at least 2-3 times at a steady pace. If taps are too far apart, tap detection restarts from the latest tap.

## Project Optimization

- **Sample Pool:** Opens the sample import browser specifically for the project's local sample directory. This allows you to manage and import samples directly into your project's folder.
- **Remove Unused Samples:** Scans the project's sample folder and removes any audio files that are not currently used by any instrument in the project.
- **Rebuild Sample Cache:** Reloads every sample of the current project from the SD card. Use it if a sample is reported as not fitting even though the pool is nearly empty, or after sample files were changed on the card by a computer. Not available during playback, and reload the project afterwards. See [Sample cache](#sample-cache).
- **Remove Unused Instruments:** Resets all instruments that are not used in any phrase to their default state and removes their associated samples from the project.

## Sample cache

A project's samples are copied into the picoTracker's internal flash when the project loads, which is what makes playback fast. Where each sample ended up is written to a small file on the SD card (`/.current.samples`), so that loading the same project again - including after a power cycle - skips copying the samples and is near instant.

That file is only used when it can be trusted:

- It belongs to one project, by name. Loading a different project ignores it.
- Every sample it lists must still exist on the card with the same file size, and no extra samples may have appeared. Replacing, resizing, adding or removing a `.wav` file in the project's `samples` folder makes the next load read the samples from the card instead - slower once, and the cache is then refreshed.
- A firmware update that moves or changes the sample area invalidates it.

Two consequences worth knowing:

- **Sample edits need a project reload.** Trimming or normalising a sample in the [Sample Editor](sample-editor.html) writes the new WAV to the card, but the pool keeps playing the old audio held in flash until you reload the project. Reload it before importing more samples or loading another project.
- **Removing a sample** in the [Project Pool](projectpool.html) deletes the file from the card, while the pool keeps the sample until the project is reloaded.

Samples are only ever *appended* to flash, so removing and re-importing grows the space they use. It is reclaimed by **Rebuild Sample Cache**, by reloading the project, or by rebooting.

## Project Management

- **project:** Displays the current name of the project. To edit the name:
  - Hold **ENTER** and use **UP** / **DOWN** to change the character at the cursor.
  - Hold **ENTER** and use **LEFT** / **RIGHT** to move the cursor.
  - Press **RIGHT** at the end of the name to add a new character.
  - Press **EDIT** to delete the character at the cursor.
  The project name is limited to 16 characters.
- **Browse:** Go to the project file browser to load a different project.
- **Save:** Save the current project state to disk. **NOTE:** *Saving cannot be done during playback.*
- **New:** Replaces the current project with a new, blank project. A confirmation dialog will appear.
- **Random:** Generates a new, random name for the current project.

## Rendering

You can render your song to audio files (.wav) directly on the picoTracker. Rendering starts from the last played row in the Song screen and continues until the song ends.

**Note:** Rendering is only possible if the first row of your song (Row 00) contains at least one phrase.

- **Mixdown:** Renders the entire song to a single stereo master file in the `/renders` folder on your SD card.
- **Stems:** Renders each of the 8 channels to separate audio files (stems) simultaneously in the `/renders` folder.

During rendering, a progress bar is shown. You can cancel the process at any time by pressing **OK**. For more details, see the [Renders and Stems](renders.html) page.
