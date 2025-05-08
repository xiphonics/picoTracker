---
title: Project Management
template: page
---

![screen capture of project screen](image/project-screen-small.png)

On the project screen you change various settings of the current project, save the current project, rename it (including giving it a random new name) create a new blank project or go to the project browser screen to **load** an another existing project.

Your current project settings are saved automatically every minute except when the sequencer is running, ie. when the current project is playing. This means that should you restart the picoTracker or accidently power off or a crash occurs, your current project state within the last minute will be restored when you restart the picoTracker.

You can ***explicitly*** save the current project by pressing [SAVE] on the project screen. By doing this you can then later on revert to the state that you just saved by reloading the current project using the [Load] button on screen button on the project screen.

## Current Project settings

- **Tempo:**: Can be set between 60bpm [0x3c] and 400bpm [0x190]. Resolution aligned to LSDJ.
- **Master:** Main volume goes from 10% to 200%.
- **Transpose:** Live transposition of every triggered instruments.
- **Scale:** Set the scale that will applied to all notes entered in the project. When entering a note in the Phrase screen, you will only be able to enter notes that belong to the selected scale. See [the reference](scales.html) for a list of all available scales.

## Current Project optimisation

- **Compact Instruments:** All unused instruments will have their sample set to (null) and the sample file will be removed from the projects sample subdirectory.

## Project Management

- **project:** Displays the current name of the project and allows you to edit it
- **Load** Go to the project file browser to load a different project or reload the last explicitly saved version of the current project
- **Save** Save the current project **NOTE:** *saving currently cannot be done during playback.*
- **New** *REPLACE* the current project with a new, *Blank* project.  
- **Random** *RENAME* the current project with a new, *Randomly generated* name.
- **Import Sample** Access the sample import file browser with a single press of the `ENTER` key. When you're done in the Import Screen, you can press `NAV`+`LEFT` to return to the Project Screen.

The project name is **limited to 16 characters**. 

You edit to project name by moving onto the name field and then holding the `ENTER` key while using the `UP` and `DOWN` keys to change the selected character and `LEFT` and `RIGHT` keys to move the cursor to the left or right of the current character. When on the last character, you can add chararacter to the end of the project name by using the `RIGHT` key.
To delete a character, select the character and press `EDIT`. 