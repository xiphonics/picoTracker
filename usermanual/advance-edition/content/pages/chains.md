---
title: Chain Screen
template: page
---

![screen capture of chain screen](image/chain-screen-small.png)

- The three columns of the phrase screen, from left to right: (red) row counter, phrase list, and transpose.
- Chains are made up of phrases. Each hex number in the second column represents a phrase.
- You can navigate to any of those phrases by putting your cursor on one and hitting <span class="minikeys">ALT</span>+<span class="minikeys">RIGHT</span>.
- The rightmost column is transpose. If you use very high numbers like FF the phrase on that row will transpose down. Low numbers like 04 will cause the phrase on that row to transpose up.
- you can jump to previous / next chain on the row with with <span class="minikeys">EDIT</span>+<span class="minikeys">LEFT</span>/<span class="minikeys">RIGHT</span>
- You can make a new phrase by hitting <span class="minikeys">EDIT</span> <span class="minikeys">EDIT</span> on a blank space in the Chain screen.
- You can clone a phrase by highlighting it with the cursor and pressing <span class="minikeys">ALT</span>+(<span class="minikeys">EDIT</span>, <span class="minikeys">ENTER</span>).
- You can copy a phrase/transposition/selection by highlighting and pressing <span class="minikeys">EDIT</span>.
- You can cut or delete a phrase/transposition/selection by highlighting and pressing <span class="minikeys">EDIT</span>+<span class="minikeys">ENTER</span>.
- Make a big selection by pressing <span class="minikeys">NAV</span>+<span class="minikeys">EDIT</span>, then Arrows around to highlight.
- Press <span class="minikeys">NAV</span>+<span class="minikeys">ENTER</span> to paste.
- With a selection active, press <span class="minikeys">ENTER</span>+<span class="minikeys">PLAY</span> to resample the selected chain rows.
- Chain resampling starts from the first selected phrase row, renders the selected rows in song context, writes `resample.wav` into the current project's samples folder, and loads it into the sample pool.
- In Song mode, <span class="minikeys">Play</span> starts/stops playback from the highlighted step, soloing the current chain
- In Live mode <span class="minikeys">Play</span> queues the currently highlighted phrase step
