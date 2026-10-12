---
title: Chain Screen
template: page
---

![screen capture of chain screen](image/chain-screen-small.png)

- The three columns of the phrase screen, from left to right: (red) row counter, phrase list, and transpose.
- Chains are made up of phrases. Each hex number in the second column represents a phrase.
- You can navigate to any of those phrases by putting your cursor on one and hitting `ALT`+`RIGHT`.
- The rightmost column is transpose. If you use very high numbers like FF the phrase on that row will transpose down. Low numbers like 04 will cause the phrase on that row to transpose up.
- you can jump to previous / next chain on the row with with ``EDIT`+`LEFT`/`RIGHT`
- You can make a new phrase by hitting `EDIT EDIT` on a blank space in the Chain screen.
- You can clone a phrase by highlighting it with the cursor and pressing `ALT + (EDIT, ENTER)`.
- You can copy a phrase/transposition/selection by highlighting and pressing `EDIT`.
- You can cut or delete a phrase/transposition/selection by highlighting and pressing `EDIT`+`ENTER`.
- Make a big selection by pressing `NAV`+`EDIT`, then Arrows around to highlight.
- Press `NAV`+`ENTER` to paste.
- In Song mode, `Play` starts/stops playback from the highlighted step, soloing the current chain
- In Live mode `Play` queues the currently highlighted phrase step
Cloning a phrase or pressing ENTER twice to create a new phrase chooses the first unused number above the highlighted phrase. On an empty cell, the search starts above the last entered phrase. The search continues across hexadecimal blocks (for example, `1F` to `20`) and wraps to `00` if no higher number is free. If all numbers are used, the cell stays unchanged.
