# Theme Color Mapping By Screen

This document maps theme color **names** (as configured in Device -> Theme settings) to where they are used in the UI.

## Theme Color Names -> Internal Tokens

| Theme name (config/UI) | Internal `ColorDefinition` | Default RGB |
|---|---|---|
| `BACKGROUND` / `Background` | `CD_BACKGROUND` | `0x0F0F0F` |
| `FOREGROUND` / `Foreground` | `CD_NORMAL` | `0xADADAD` |
| `HICOLOR1` / `HiColor1` | `CD_HILITE1` | `0x846F94` |
| `HICOLOR2` / `HiColor2` | `CD_HILITE2` | `0x6B316B` |
| `CONSOLECOLOR` / `Console` | `CD_CONSOLE` | `0xFF00FF` |
| `CURSORCOLOR` / `Cursor` | `CD_CURSOR` | `0x776B56` |
| `INFOCOLOR` / `Info` | `CD_INFO` | `0x29EE3D` |
| `WARNCOLOR` / `Warn` | `CD_WARN` | `0xEFFA52` |
| `ERRORCOLOR` / `Error` | `CD_ERROR` | `0xE84D15` |

Notes:
- If `CURSORCOLOR` is not set, cursor color is initialized from `FOREGROUND`.
- `BACKGROUND` is used as clear color for the full screen buffer, not as normal text color.

## Shared UI Rules (Used Across Multiple Screens)

- Screen background clear: `BACKGROUND`.
- Screen map letters:
  - map baseline letters (`D P G S C P I T T`): `FOREGROUND`
  - current-screen letter highlight: `HICOLOR2`
- Battery gauge:
  - charging/high battery (`[CHG]`, `[+++]`, `[++ ]`): `INFO`
  - low battery (`[+  ]`): `WARN`
  - critical battery (`[   ]`): `ERROR`
- Generic field widgets (`UIIntVarField`, `UIBigHexVarField`, `UINoteVarField`, etc.):
  - non-focused field text: `FOREGROUND`
  - focused field text: `HICOLOR2` + inverted
- Action buttons (`UIActionField`):
  - non-focused: `FOREGROUND`
  - focused: `HICOLOR2` + inverted
- Project name text field (`UITextField`) value text: `INFO`.

## Per-Screen Mapping

## Song Screen

- `FOREGROUND`:
  - title area text (`Song`/`Live`, project name)
  - non-selected pattern cell values
  - clip indicator text (`clip` / `----`)
  - play time text (`mm:ss`)
  - non-playing note panel text
- `HICOLOR1`:
  - row numbers on the left
  - non-current channel note panel cells (via shared `drawNotes`)
- `HICOLOR2`:
  - selected block / cursor cell highlight (inverted)
  - current channel note panel cells (via shared `drawNotes`)
- `CURSOR`:
  - playback position markers and queue/live markers (`>`, `-`, live indicator chars)

## Chain Screen

- `FOREGROUND`:
  - title (`Chain xx`)
  - chain phrase values / transpose values when not selected
  - playback marker column (`>`, `-`, live indicator) in current code path
- `HICOLOR1`:
  - row numbers
- `HICOLOR2`:
  - selected cell highlight in chain/transpose columns (inverted)
- Shared footer/map/notes/battery colors apply.

## Phrase Screen

- `FOREGROUND`:
  - title (`Phrase xx`)
  - note/instrument/command/param columns when not selected
  - inline instrument name text on title row (`Ixx:<name>`)
  - playback marker column (`>`, `-`, live indicator) in current code path
  - command help legend text (top lines)
- `HICOLOR1`:
  - row numbers
- `HICOLOR2`:
  - selected cell highlight in phrase grid (inverted)
- Shared footer/map/notes/battery colors apply.

## Table Screen

- `FOREGROUND`:
  - title (`Table XX`)
  - command/param columns when not selected
  - command help legend text (top lines)
  - playback position markers (`>`) and cleared marker spaces in current code path
- `HICOLOR1`:
  - row numbers
- `HICOLOR2`:
  - selected table cell highlight (inverted)
- Shared footer/map/notes/battery colors apply.

## Groove Screen

- `FOREGROUND`:
  - title (`Groove: xx`)
  - groove values (`--` or hex)
  - current groove row is highlighted with inversion of `FOREGROUND` (not `HICOLOR2`)
  - playback marker column (`>`) and cleared marker space in current code path
- `HICOLOR1`:
  - row numbers
- Shared footer/map/notes/battery colors apply.

## Instrument Screen

- `FOREGROUND`:
  - title (`Instrument XX`)
  - all non-focused parameter labels/values and static labels
- `HICOLOR2`:
  - focused editable field highlight (inverted)
- `INFO`:
  - currently edited text-value field type (`UITextField`) when used
- Shared map/battery colors apply.

The instrument screen’s specific fields depend on instrument type (Macro/Sample/SID/MIDI/Opal), but all editable fields use the same focus/non-focus color behavior above.

## Project Screen

- `FOREGROUND`:
  - title (`Project <name>`)
  - non-focused fields (`tempo`, `master`, `transpose`, `scale`, actions)
- `HICOLOR2`:
  - focused action/number field (inverted)
- `INFO`:
  - project name text value in `project: <name>` editable text field
- Shared map/battery colors apply.

## Device Screen (Theme + System Settings)

- `FOREGROUND`:
  - title (`Device`)
  - non-focused settings fields
  - build string on bottom row
- `HICOLOR2`:
  - focused settings field (inverted)
- Theme swatches:
  - each swatch is drawn using its mapped color token (`FOREGROUND`, `HICOLOR1`, `HICOLOR2`, `CONSOLE`, `CURSOR`, `INFO`, `WARN`, `ERROR`) with inverted blocks
- Shared map/battery colors apply.

## Import Sample Screen

- `INFO`:
  - title (`Import Sample`)
- `FOREGROUND`:
  - unselected directory/file names
- `HICOLOR2`:
  - selected file/directory row (inverted)
  - selected file size footer (`[size: ...]`) (inverted)
- Battery gauge colors apply (`INFO`/`WARN`/`ERROR` depending on charge).

## Select Project Screen

- `INFO`:
  - title (`Load Project`)
- `FOREGROUND`:
  - unselected project names
- `HICOLOR2`:
  - selected project row (inverted)
- Battery gauge colors apply (`INFO`/`WARN`/`ERROR` depending on charge).

## Null/Boot Screen

- `HICOLOR2`:
  - centered build/version line at startup

## Console Screen

- `FOREGROUND`:
  - all console lines

## Modal Dialogs (MessageBox, confirmation prompts)

- `HICOLOR2`:
  - dialog border/frame (inverted blocks)
- `ERROR`:
  - dialog message text (line 1 and optional line 2)
- `FOREGROUND`:
  - button labels (`Ok`, `Yes`, `Cancel`, `No`)
  - selected button is highlighted via inversion

## Currently Unused Or Indirect

- `CONSOLE` (`CD_CONSOLE`):
  - available and fully theme-configurable
  - currently used directly in this UI path for Device screen color swatch preview
  - not currently used for standard text rendering in the main application views
