---
title:  Renders and Stems
template: page
---

![rendering dialog](image/rendering-dialog-small.png)

## Audio Rendering in picoTracker

picoTracker allows you to render your compositions to audio files, providing two different rendering options: **Mixdown** and **Stems**. These features enable you to export your music for sharing, further processing in a DAW, or archiving. The render options are available in the **Project View**.

## Accessing the Render Options

The render options can be found in the **Project View**. Navigate to the Project View and look for the "Render:" section, which contains two options:

- **Mixdown** - Creates a single stereo audio file of the selected portion of your song
- **Stems** - Creates separate audio files for each channel/instrument in your composition

***NOTE:***  Any existing files in the `/renders` directory for the current project will be **overwritten** by the rendering process.

Use `EDIT`+arrow keys to select rows in the Song, Chain, or Phrase screen. Press `ENTER`+`PLAY` on one of these screens to render the selected rows to a mixdown file. On the Project screen, **Mixdown** renders the active Song screen selection, including its selected channels.

## Rendering a Mixdown

A mixdown creates a single stereo audio file containing the selected part of your composition. This is useful when you want to share your track or use it in other applications.

To create a mixdown:

1. In the Song screen, select the rows and channels you want to render with `EDIT`+arrow keys
2. Navigate to the Project View
3. Make sure your song is not currently playing
4. Select the "Mixdown" option under "Render:"
5. The rendering process will begin, and a progress dialog will display the current render progress
6. The selected portion of the song will play through while rendering
7. You can press "OK" at any time to stop the rendering process
8. When the selection has completed playing through, the rendering will automatically finish

***NOTE:*** A Song screen selection is required when rendering from the Project screen. To render a selected region from the Song, Chain, or Phrase screen directly, use `ENTER`+`PLAY`.

***NOTE:*** The progress displayed during the rendering process shows the percentage of the song rendered.

***NOTE:*** You may hear noise or audio artifacts while the rendering is in progress or the audio being played at slower speed than expected, but this is normal due to the extra processing required during rendering and it will **not** affect the final rendered audio.

## Rendering Stems

Stems are separate audio files for each individual channel/instrument in your composition. This is particularly useful when you want to:

- Further process individual elements of your track in a DAW
- Create remixes or alternative versions of your composition
- Have more control over the final mix

To render stems:

1. Navigate to the Project View
2. Make sure your song is not currently playing
3. Select the "Stems" option under "Render:"
4. The rendering process will begin, and a progress dialog will display the current render progress
5. The song will play through until the end while rendering
6. You can press "OK" at any time to stop the rendering process
7. When the song has completed playing through, the rendering will automatically finish

## Render Output

The rendered audio files are saved to your device's SD card in the `/renders` top level directory.

- Mixdown files are named `(projectname)-mixdown.wav`
- Stem files are named `(projectname)-channel(number).wav`

## Render Quality

All audio is rendered in stereo at 44.1kHz sample rate with 16-bit depth.

## Tips for Rendering

- Make sure your composition is complete and sounds as intended before rendering
- Check that all channel volumes and the master volume are set appropriately
- For the best quality output, avoid clipping by ensuring your levels aren't too high
- If you need to stop a render in progress, press the "OK" button on the render progress dialog
- Rendering will automatically stop when the song gets to the end and will *not* loop back to the beginning as it does in the song screen

## Accessing Rendered Files

After rendering, you can access your files in the `/renders` directory on your device's sdcard by taking out the sdcard and using a computer or other device to access it.
