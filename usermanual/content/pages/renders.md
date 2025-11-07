---
title:  Renders and Stems
template: page
---

![rendering dialog](image/rendering-dialog-small.png)

## Audio Rendering in picoTracker

picoTracker allows you to render your compositions to audio files, providing two different rendering options: **Mixdown** and **Stems**. These features enable you to export your music for sharing, further processing in a DAW, or archiving. The render options are available in the **Project View**.

## Accessing the Render Options

The render options can be found in the **Project View**. Navigate to the Project View and look for the "Render:" section, which contains two options:

- **Mixdown** - Creates a single stereo audio file of your entire composition
- **Stems** - Creates separate audio files for each channel/instrument in your composition

***NOTE:***  Any existing files in the `/renders` directory for the current project will be **overwritten** by the rendering process.

## Rendering a Mixdown

A mixdown creates a single stereo audio file containing your entire composition. This is useful when you want to share your completed track or use it in other applications.

To create a mixdown:

1. Navigate to the Project View
2. Make sure your song is not currently playing
3. Select the "Mixdown" option under "Render:"
4. The rendering process will begin, and a progress dialog will display the current render time
5. The song will play through once from beginning to end while rendering
6. You can press "OK" at any time to stop the rendering process
7. When the song has completed playing through, the rendering will automatically finish

***NOTE:*** The time displayed during the rendering process is the elapsed time of the audio being rendered, not the actual "clock" time of how long it takes to perform the render.

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
4. The rendering process will begin, and a progress dialog will display the current render time
5. The song will play through once from beginning to end while rendering
6. You can press "OK" at any time to stop the rendering process
7. When the song has completed playing through, the rendering will automatically finish

## Render Output

The rendered audio files are saved to your device's sdcard int he `/renders` top level directory. The exact location and format of the files depends on your picoTracker configuration:

- Mixdown files are named `(projectname)-mixdown.wav`
- Stem files are named `(projectname)-channel(number).wav`

## Render Quality

All audio is rendered at 44.1kHz sample rate with 16-bit depth.

## Tips for Rendering

- Make sure your composition is complete and sounds as intended before rendering
- Check that all channel volumes are set appropriately
- For the best quality output, avoid clipping by ensuring your levels aren't too high
- If you need to stop a render in progress, press the "OK" button on the render progress dialog
- Rendering will automatically stop when the song gets to the end and will *not* loop back to the beginning as it does in the song screen

## Accessing Rendered Files

After rendering, you can access your files in the `/renders` directory on your device's sdcard by taking out the sdcard and using a computer or other device to access it.
