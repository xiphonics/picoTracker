---
title: Tables
template: page
---

![screen capture of table screen](/image/table-screen-small.png)

- Hopping to self in table holds the step for x ticks. For example

05 HOP 1005
will hold step 5 for 10 ticks. Interesting for example after a VOL command to allow complex envelopes:
00 VOL 0400 ; starts short volume decay to zero 01 HOP 1010 ; holds enough ticks for VOL to complete 02 VOL 0560 ; raise volume to 60 03 HOP 6003 ; hold for a long time to allow VOL to ; complete and hold volume to 60

- After Hopping count is reached, the table pointer moves directly to the line after

the HOP rather than staying on the HOP line for one tick. Makes more sense.
