---
title: MIDI Control Surfaces
template: page
---

## Overview

picoTracker Advance allows the use of USB MIDI control surfaces in addition to regular MIDI note input. When a recognised control surface is connected, picoTracker takes over the device when the Advance is placed into **Live mode** on the **Song screen**, using it as a hands-on live launcher and mixer.

### Compatible MIDI USB Devices  (As of June, 2026)

***NOTE:*** The USB devices listed below have been tested for compatibility with the Advance, however xiphonics does not guarantee
proper operation of such devices with the Advance. Xiphonics also takes no responsibility for any damage or inconvenience caused by the use of them.

* The specification of the USB devices listed below is subject to change for certain reasons of the manufacturers. Please note that they may not operate properly with Advance depending on the changes.
* For detailed specification and information on the USB devices listed below, please contact their respective manufacturers.

The only compatible MIDI controllers for the control surface feature are: 

* [Akai APC mini mk3](https://www.akaipro.com/mpk-mini-mk3/)
* [Novation LaunchPad mk3](https://novationmusic.com/products/launchkey-mini-mk3). 

The only compatible USB Hubs that can be used with the above controls to connect to the Advance are:

* [Acer USB C to USB A Splitter 4 Port](amazon.com.au/dp/B0CS2PNXNM)
* [UGREEN USB C Hub 4 Ports](https://www.amazon.com.au/dp/B07PY87TBD?th=1)


## Akai APC Mini Mk2

The APC Mini Mk2 is the most fully featured control surface for picoTracker Advance. It provides the most complete hardware integration with a generous layout of pads, faders, and transport controls.

### Connection

Connect the APC Mini Mk2 directly to the picoTracker Advance USB port using a standard USB-C to USB-B cable or via a compatible USB Hub (see above). The APC Mini Mk2 is self-powered and works natively in USB Host mode on the Advance. No additional adapters are required.

### Grid Mapping

The APC Mini Mk2's **8x8 pad grid** maps directly to the first 8 song channels and the first 8 song rows:

* APC columns 1–8 map to song channels 1–8
* APC rows map to song rows 00–07

Pressing a pad queues that song step on the corresponding channel in Live mode.

### Faders

The APC Mini Mk2 features **9 faders**, which provide direct control over the project mixer:

* **Faders 1–8** control the volume of song channels 1–8 respectively
* **Fader 9** (the master fader on the right) controls the master output volume

Fader movements are reflected in real-time on the picoTracker display.

### Track Buttons (Scroll Controls)

The 4 arrow buttons below the grid serve as song window scroll controls in Live mode:

| Track Button | Function |
|---|---|
| Up | Scroll song window up one row (fine up) |
| Down | Scroll song window down one row (fine down) |
| Left | Scroll song window up one page (page up) |
| Right | Scroll song window down one page (page down) |

### Scene Buttons (Launch + Shift)

The 8 scene buttons on the right side of the grid provide song row launching:

* **Scene buttons 1-8** launch the corresponding song row (all 8 channels on that row)

Additionally **with Shift held:**
* **Scene button 8** (bottom-right) acts as the transport button — toggles Live playback start/stop

### LED Feedback

The APC Mini Mk2's RGB pads provide rich visual feedback:

* **Queued steps** are shown in yellow
* **Currently playing steps** are shown in pulsing green
* **Inactive cells** are dimmed
* Colors are theme-aware and adapt to the current Advance theme
* The scene buttons illuminate to indicate row launch state

### Song Window Indicator

When a control surface is active, the Song screen displays a visual indicator on the left side of the row numbers showing the portion of the song window currently mapped to the control surface grid. This helps you understand which rows are accessible from the hardware pads.

## Novation Launchpad Mini Mk3 (Supported with Caveats)

The Launchpad Mini Mk3 is also compatible as a control surface, but with some important caveats to be aware of.

### Caveat: USB Adapter Required

The Launchpad Mini Mk3 uses a **USB-C connector**, so can be connected to the picoTracker Advance's USB port via a USB-C to USB-C cable.

{% callout type=warn | Make sure that you use a usb-c cable that supports data and not just charging!  %}

### Grid Mapping

The Launchpad Mini Mk3's **8x8 grid** maps directly to the first 8 song channels and the first 8 song rows:

* Launchpad columns map to song channels 1–8
* The top Launchpad row is song row 00
* The bottom Launchpad row is song row 07

Pressing a grid pad queues that song step on that channel in Live mode.

### Playback Controls

The following Launchpad controls are currently supported:

* The **8x8 grid** queues individual channel steps
* The **Top 7** only **right-hand scene buttons** queue full song rows
* The **top row buttons** provide scroll controls and transport:
  * Up button — Scroll song window up one row (fine up)
  * Down button — Scroll song window down one row (fine down)
  * Left button — Scroll song window up one page (page up)
  * Right button — Scroll song window down one page (page down)
  * User button — Toggles Live playback start/stop

### Channel Mute Control

The bottom-right button (Solo/Mute) on the right-hand column acts as a **mute modifier**.

While holding the mute modifier, press a pad on the **top row** above the 8x8 grid to act on that channel:

* Press and hold the mute modifier
* Press a top-row grid pad to mute that channel
* If you release the grid pad first while still holding the modifier, the mute is reverted (momentary mute)
* If you release the modifier first, the mute stays latched and becomes a toggle mute
* Repeating the same latched mute gesture on a muted channel will unmute it

This gives you both **momentary mute** and **toggle mute** behaviour, matching the same release-order logic as the onboard Song screen mute combo.

### Transport Control

* **User Button** acts as the transport button — toggles Live playback start/stop

### LED Feedback

The Launchpad LEDs reflect the current live state:

* Queued steps are shown in yellow
* Currently playing steps are shown in pulsing green
* While the mute modifier is held, the grid column shows (red) mute state for each channel

## General Notes

### When Control Surface is Active

Control surface integration is active only when the Advance is in **Live mode** and compatible USB control surface isconnected and detected.

### Song Window Sync

Both controllers support automatic song window scrolling to keep the visible portion of the song aligned with the currently playing position. The grid display on the Song screen shows which rows are mapped to the hardware controller, helping you understand the current window position.

### Limitations

Current control surface support is intentionally focused:

* It is only active in **Live mode**
* Fader control on the APC Mini Mk2 is limited to channel and master volume
* Not all buttons are assigned (some are reserved for future features)
