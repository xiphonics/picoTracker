---
title: Device Management
template: page
---

The device screen allows you to manage settings for the picoTracker device hardware itself.

**Note:** the picoTracker saves its device settings to the SDCard so if you replace the sdcard in your picoTracker, the device configuration will reset to factory defaults.

![screen capture of device screen](image/device-screen-small.png)

## MIDI

- **MIDI device:** Lists the picoTracker MIDI interfaces available. Available options are:
    * None
    * TRS
    * USB
    * TRS+USB
- **MIDI sync:** Enable/disable MIDI clock sync messages being sent by the picoTracker.

## Line Out Mode (1st Edition RP2040 model only)

On the first edition picoTracker, the audio output can be set to one of 3 amplification levels to make the audio level output more useful for Line Level output or Headphone output. The 3 possible settings are: 
* `HP Low`
* `HP High`
* `Line Level`

**NOTE: A reboot of the picoTracker is required to apply the newly changed audio output level!** 

**PLEASE exercise caution when using earphones or headphones!*

## Master Level (Advance model only)

On the picoTracker Advance, the Device screen exposes a **Master level** slider. This adjusts the TLV320 codec's digital DAC volume, giving you smooth control from silence up to 0 dB. Changes take effect immediately without needing to reboot.

## Line Out Mode (Advance model only)

The Advance also includes a **Line Out Mode** selector that controls the TLV320's analog output driver gain. Available settings are:

* `-6 dB`
* `0 dB`
* `+6 dB`
* `+18 dB`
* `+24 dB`
* `+28 dB`

Use the lower settings for sensitive headphones or nearfield monitors, and only step up to the higher gain modes when you need additional headroom to drive passive speakers or high-impedance inputs.

## Remote UI

This setting enables or disables sending commands to a computer attached via USB to the picoTracker running a "remote UI" application that can mirror the display from the picoTracker.

The [official picoTracker remote UI web application is available ](https://ui.xiphonics.com), it works only with Chromium based browsers.

## Display Brightness

The display brightness setting allows you to adjust the backlight level of the picoTracker's LCD display. The brightness can be set from `05` (minimum brightness) to `ff` (maximum brightness). The default value is `80` (medium brightness).

Adjusting the brightness can help with battery life (lower brightness uses less power) and visibility in different lighting conditions.

## Theme Settings

The Theme Settings screen allows you to customize the appearance of the picoTracker interface, including fonts and colors. To access the Theme Settings screen, select the "Theme settings" option on the Device screen.

See the [Theme Settings](theme.html) chapter for more details.

## Updating Firmware

The picoTracker firmware can be easily updated by powering the picoTracker via a USB connected to a PC. Select `Update firmware`. This will reboot the device into *BOOTSEL* mode, and you'll see the picoTracker show up as a USB mass storage device on the connected computer. At that point, just copy the UF2 firmware file into the USB device as you would with a USB stick or USB drive. 

Once copying (flashing) of the new firmware is completed, the picoTracker will automatically reboot and be running the newly installed firmware.
