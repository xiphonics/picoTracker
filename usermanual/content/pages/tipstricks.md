---
title: Tips and Tricks
template: page
---

These tips and tricks come [courtesy of the LGPT community](https://github.com/djdiskmachine/LittleGPTracker/blob/master/docs/wiki/tips_and_tricks.md). 

The command names have been changed to match picoTracker's naming convention and some sections have been edited for clarity.

----

## Delays and Echoes 

### Simulating LSDj's D command

One command that is from LSDj but doesn't exist in picoTracker is the [D]elay command. However, it is possible to emulate it. It's a little tricky but gives a good view of several commands so I'll explain it here:

For this, we'll need to use `POF` (play offset). `POF` is the command that allows to position the sample playback cursor both/either to an absolute point in the sample or relative to the current play position. For example, `POF 8000` will put the playback head to the middle of the sample, `POF 4000` to the first quarter. `POF 0040` will jump ahead from current position of a offset equivalent to a quarter of the sample size.

The basic trick it to use pof to re-start a running sample from position zero. Normally, the syntax should be `POF 0000` but in order to keep things tidy, this value has no effect. So the trick is to use POF to jump to position 01 the relative backward of 01, which has the same effect. So `POF 01FF` (or `POF FF01`) will have the effect or positioning the sample to position zero.

Using that in a table in combination with VOL, you can:

* turn the `VOL` to zero
* wait for x ticks
* position the sample back to zero and VOL back to normal.

This will have the same effect as a Dx command in LSDj.

So, link with the instrument a table containing:

``` 
00 VOL 0000 ---- 0000 ---- 0000
01 VOL 0080 POF  01FF ---- 0000
02 HOP 0002 HOP  0002 ---- 0000
```

Will do the trick :)

Note: if you are using very short samples, it might be that before one tick, the sample is finished and the voice is automatically killed (to save CPU). In that case, POF'ing it will have not effect. So, in general, when using re-positioning with `POF`, it is better to put a short loop (for example of 1 sample long at the end) in the instrument definition.

## Doing Echoes
Using the 'retriggering' system we just saw, we can easily emulate echoes without having to enter notes at each step. Since doing `POF 01FF` will restart the sound from it's beginning we can do the following table :

```
00 POF 01FF ---- ---- VOL 2000
01 ---- ---- ---- ---- HOP  0001
02 ---- ---- ---- ---- ---- ----
03 HOP  0000 ---- ---- ---- ----
```

The first column will generate repeating echoes (change the HOP line to adapt the echo's length) while the last one will provide a decay on the instrument volume. You can also add `FCT` command to get the filter to damp more the hi-frequencies, emulating the loss of regular delays.

Note that you can also emulate triplet using this technique.

## Oscillator mode

### Oscillator base

Oscillator is a special looping mode that allows to generate timbre from basically anything. It basically the loop start and loop end point of the instrument settings and adapts the sample scanning speed so that it becomes pitched to the played note.

The most straightforward use is when you have short waveform samples (a single square wave for example). Rather than having to compute the sample length or try to find the note that is played when looped, just turn the instrument's loop mode to 'oscillator' and it'll be automatically tuned. Of course, nothing prevents you to grab loops in ANY kind of sample, voices, drum loops wathever. The oscillator mode is a great way to get timbres from material you already got.

### Wavetables

Wavetables are a special process to allow to alter oscillator's content by switching or scanning the waveform of the oscillator while the note is playing. To get that effect, build a sample containing several waveforms (of the same loopsize preferably) in the same sample. For example, a few waves from the musicline pack. Each wave is 0x100 sample long so setup a loop from 0x00 to 0x100. Now to switch to the next wave, all you have to do is to offset the loop of 0x100. This can be easily achieved by using:

```
LOF 0100
```

Alternatively, you can also scan slowly from one wave to the other by doing a table
```
00 LOF 0001 ---- ---- ---- -----
01 HOP  0000 ---- ---- ---- -----
```
This trick works also very neatly to change the timbre of some notes only by doing single LOF commands in the phrase view, changing slightly (depending on the value used) the timbre for each note.

### Pulse Width Modulation

Pulse width modulation (PWM) is a classic effect found in synths to adapt the duty cycle of a square wave. Using the oscillator mode and `LOF`, you can achieve some coarse PWM. Take a square wave of 0X100 length (use the oscillator mode), and imagine a window of half the wave size (0x080) travelling over the wave. If the 0x80-size loop is at the beginning, you get a square sample with duty cycle 0. If centered (0x40), you get a square wave and if all ther way at the end, you get a duty cycle of 100%.

So taking a square waveform of 0x100 length, applying a loop of 0x80 in oscillator mode on it and adapting the loop position using `LOF`, we can achieve PWM quite easily :)

### Drones

Using the principle of wave scanning, you can take a long sample (a whole musical phrase for example), setup a short loop in oscillator mode on it and scan slowly accross it. This will result into a timbre that is always tuned but evolves very slowly. Try using the same instrument on different channel playing a chord for very ghostly effect.

See for example the [PeteyDroney](http://www.hexawe.net/lgptwiki/lgpt-%20PeteyDroney.rar) sample

## Breakz !

### Loop choppin' (old method)

Loop chopping is really easy in picoTracker through the `POF` command. POF sees the whole sample as 256 chunks of the same length. It allows to put the current playback head to the beginning of any of those chunks. To get it, we'll take a really simple drum loop sample going like this:

```
BD1 -- SD1 -- BD2 HH1 SD2 -- 
```
BD2 is in the middle of the loop to to jump there, you issue `LOF 8000` SD1 is at the first quater of the loop, use `LOF 40000` HH1 is at `LOF A000` SD2 is at `LOF C000`

You can see by combining a lot of `LOF` commands one after the other, you can re-order the hits of the drum loop in pretty much any order and with quite a bit of resolution. Depending on the BPM and the style of the original wave, you'll end up with breakbeat/jungle/breakcore or just plain ol' mess.

Here's a nice example, courtesy of jonbro, chopping drums he had recorded previously:

[Jonbro - the thing is the thing](https://battleofthebits.com/arena/Entry/the+thing+is+the+thing/1479/)

### Slice! (new chopping method)

Since version 2.1 there is a new loop mode called Slice.

Using this, you can assign up to 256 individual slices from C-2 (C minus two) up to the amount of slices you set. How handy!

Of course, you are not limited to drum loops. Just chop anything away !

## Grooves, BPM, Math and You

If you've ever messed around with grooves in either picoTracker or LSDJ, you've probably noticed that changing the total number of tics in a groove/pattern will change the perceived BPM of your track. Purists will say this is a new time signature, but in many cases, it will still sound like 4/4 time, but at a different tempo. So, if you're trying to sync by ear to a basic drum machine incapable of changing tics per beat or even loading a rendered WAV into a DAW, the following formula is for you:

`(96*projectBPM)/total pattern tics= relative BPM`

The constant of proportionality in this case, 96, comes from picoTracker's default behaviour where a 16 step pattern with each step being 6 tics will give you the BPM set in the project screen, where one beat equals 4 steps. So, 16 steps * 6 tics will give you 96 tics per pattern.

*Example*
You make a song with a groove of 4/4. This groove will play 8 times in one pattern since it's 2 steps long. So, total pattern tics = `(4 tics + 4 tics)*8= 64` tics.

The project screen says the BPM is 100.

`(96*100)/64 = 150` is the perceived BPM