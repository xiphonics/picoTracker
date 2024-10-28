---
title: Device Management
template: page
---

The device screen allows you to manage settings for the picoTracker device hardware itself.

**Note:** the picoTracker saves its device settings to the SDCard so if you replace the sdcard in your picoTracker, the device configuration will reset to factory defaults.

![screen capture of device screen](/image/device-screen-small.png)

## MIDI

- **MIDI device:** Lists the picoTracker MIDI interfaces available. Available options are: None, TRS, USB, TRS+USB
- **MIDI sync:** Option to enable different settings for MIDI clock sync messages output.

## Line Out Mode (1st Edition RP2040 model only)

On first edition picoTrackers, the audio output can be set to one of 3 amplification levels to make the audio level output more useful for Line Level output or Headphone output. The 3 possible settings are: 
* `HP Low`
* `HP High`
* `Line Level`

**NOTE: A reboot of the picoTracker is required to apply the newly changed audio output level!** 

**PLEASE exercise caution when using earphones or headphones!*

## Remote UI

This setting enables or disables sending commands to a computer attached via USB to the picoTracker running an a "remote UI" application that can mirror the display from the picoTracker.

## Font

The font used for the picoTracker user interface. Choice of the "standard" or "bold" font.

## Color Themes

The color scheme for the picoTracker can be customised and saved. 

The following colors can be configured and are listed with their default setting as well as a note on the colors use in the picoTracker user interface:

| Name | Default | Use
| -------- | ------- | -------
BACKROUND|0F0F0F| Background color on all screens
FOREGROUND|ADADAD|text and cursor in cursor
HICOLOR1|846F94|row count in song screen
HICOLOR2|6B316B|inverted highlight, eg. "Song" screen label 
INFOCOLOR|33EE33|information displays eg. used for battery gauge ok level
WARNCOLOR|11EE22|warning displays eg. battery gauge low level
ERRORCOLOR|FF1111|error displays eg. battery gauge critical level
CURSORCOLOR|224400|??
CONSOLECOLOR|99FFAA|??

## Updating Firmware

The picoTracker firmware can be easily updated by powering the picoTracker via a USB connected to a PC. Select `Update firmware`. This will reboot the device into *BOOTSEL* mode, and you'll see the picoTracker show up as a USB mass storage device on the connected computer. At that point, just copy the UF2 firmware file into the USB device as you would with a USB stick or USB drive. 

Once copying (flashing) of the new firmware is completed, the picoTracker will automatically reboot and be running the newly installed firmware.
