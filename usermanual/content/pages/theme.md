---
title: Theme Settings
template: page
---

The Theme Settings screen allows you to customize the appearance of the picoTracker interface, including fonts and colors.

![screen capture of theme settings screen](image/theme-screen-small.png)

## Accessing the Theme Settings

To access the Theme Settings screen, select the "Theme settings" option on the Device screen. You can return to the Device screen at any time by pressing the **NAV+LEFT** key combination.

## Font

The font used for the picoTracker user interface. You can choose between:
* **standard** - The default font
* **bold** - A bolder, more prominent font

When you change the font, the entire interface will update immediately to reflect your choice.

## Color Themes

The picoTracker allows you to customize the color scheme of the interface. Each color is represented as a hexadecimal RGB value (e.g., FFFFFF for white). You can modify any of the following colors:

| Name | Default | Use |
| ---- | ------- | --- |
| Foreground | ADADAD | Normal text and UI elements |
| Background | 0F0F0F | Background color on all screens |
| Highlight1 | 846F94 | Primary highlight color, used for row counts in song screen |
| Highlight2 | 6B316B | Secondary highlight color, used for inverted highlights like screen labels |
| Console | FF00FF | Console text color |
| Cursor | 776B56 | Cursor color |
| Info | 29EE3D | Information displays, used for battery gauge OK level, vu meter normal level |
| Warning | EFFA52 | Warning displays, used for battery gauge low level, vu meter high level |
| Error | E84D15 | Error displays, used for battery gauge critical level, vu meter clipping level |
| Accent | 00FF00 | Primary accent color, used for playing track indicators and alternating row highlights |
| AccentAlt | FF0000 | Secondary accent color, used for muted track indicators, alternating row highlights, and `FE` chain values in Song View |
| Emphasis | FFA500 | Used for emphasis elements like major beat (quarter note) indicators in the Phrase view and `00` chain values in Song View |

Note: Accent and AccentAlt are used to mark every block of 4 rows in each of the "grid views", currently this row count value is *not* a user setting.

## Color Swatches

Next to each color setting, a small color swatch is displayed to show you the current color. This makes it easier to visualize your changes as you adjust the colors.

## Saving Changes

All changes to the theme settings are automatically saved to the SD card as you make them. This ensures your customized theme will be preserved even when you power off the picoTracker.

## Importing and Exporting Themes

picoTracker allows you to import and export theme files (.ptt), making it easy to share your custom themes with others or back up your favorite color schemes.

### Exporting a Theme

1. Navigate to the Theme settings screen
2. Press the **EXPORT** button (or **NAV+EXPORT** if using a different control scheme)
3. Enter a name for your theme using the on-screen keyboard
4. Press **ENTER** to confirm

Your theme will be saved as a `.ptt` file in the `themes` directory on your SD card. If a theme with the same name already exists, you'll be asked if you want to overwrite it.

### Importing a Theme

1. Navigate to the Theme settings screen
2. Press the **IMPORT** button (or **NAV+IMPORT** if using a different control scheme)
3. Browse the file list to find the `.ptt` file you want to import
4. Select the file and press **PLAY** to import it

The imported theme will be immediately applied to the interface. The theme name will be updated to match the imported file's name (without the .ptt extension).

### Theme File Location

- Exported themes are saved in: `/themes/[theme-name].ptt`
- When importing, the system will look for `.ptt` files in the root directory and any subdirectories

## Tips for Creating Custom Themes

* Be **VERY** careful when changing the foreground and background colors to not use the same or very similar color for both! 
* Create themes with good contrast between text and background colors for better readability
* Consider using complementary colors for highlights to make important elements stand out
* If you're performing in low-light conditions, consider using darker background colors to reduce screen glare
* For better visibility in bright environments, use higher contrast between foreground and background colors
* When sharing themes, consider including the theme name in the filename for easy identification
