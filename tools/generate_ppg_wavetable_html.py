#!/usr/bin/env python3
"""Generate an HTML overview of PPG wavetable frames as waveform thumbnails."""

# running using: python3 tools/generate_ppg_wavetable_html.py --output docs/ppg_wavetables_overview.html

from __future__ import annotations

import argparse
import html
import re
from pathlib import Path
from typing import Iterable


FRAME_MARKER_RE = re.compile(r"//\s*frame\s+([0-9A-Fa-f]{2})\s*,\s*sample\s+000\b")
ARRAY_RE = re.compile(
    r"static\s+const\s+uint8_t\s+(\w+)\s*\[\]\s*=\s*\{(?P<body>.*?)\};",
    re.DOTALL,
)
NUMBER_RE = re.compile(r"\b\d+\b")


def strip_line_comments(text: str) -> str:
    return re.sub(r"//.*", "", text)


def parse_wavetable_header(path: Path) -> dict:
    text = path.read_text(encoding="utf-8")
    array_match = ARRAY_RE.search(text)
    if not array_match:
        raise ValueError(f"No uint8_t wavetable array found in {path}")

    body = array_match.group("body")
    frame_markers = [int(m.group(1), 16) for m in FRAME_MARKER_RE.finditer(body)]
    if not frame_markers:
        raise ValueError(f"No frame markers found in {path}")

    numeric_body = strip_line_comments(body)
    values = [int(n) for n in NUMBER_RE.findall(numeric_body)]
    if not values:
        raise ValueError(f"No sample data found in {path}")

    frame_count = len(frame_markers)
    if len(values) % frame_count != 0:
        raise ValueError(
            f"Sample count {len(values)} is not divisible by frame count {frame_count} in {path}"
        )

    samples_per_frame = len(values) // frame_count
    frames = [
        values[i * samples_per_frame : (i + 1) * samples_per_frame]
        for i in range(frame_count)
    ]

    return {
        "path": path,
        "array_name": array_match.group(1),
        "frame_markers": frame_markers,
        "frame_count": frame_count,
        "samples_per_frame": samples_per_frame,
        "frames": frames,
    }


def waveform_svg_points(
    samples: Iterable[int],
    width: int = 132,
    height: int = 88,
    x_margin: float = 1.0,
    y_margin: float = 6.0,
) -> str:
    vals = list(samples)
    if not vals:
        return ""

    x_span = max(1.0, width - 2 * x_margin)
    y_span = max(1.0, height - 2 * y_margin)
    n = max(1, len(vals) - 1)
    pts: list[str] = []
    for i, sample in enumerate(vals):
        x = x_margin + (x_span * i) / n
        # uint8 waveform data: map 0..255 to svg top..bottom and invert for plot
        y = y_margin + y_span * (1.0 - (sample / 255.0))
        pts.append(f"{x:.2f},{y:.2f}")
    return " ".join(pts)


def render_html(wavetables: list[dict], columns: int) -> str:
    parts: list[str] = []
    parts.append(
        """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>PPG Wavetable Waveform Overview</title>
<style>
:root {
  --bg: #f3f3f3;
  --panel: #ffffff;
  --ink: #1f1f1f;
  --muted: #666;
  --accent: #c85f5f;
  --grid-line: #d8d8d8;
  --wave: #222;
  --cell-border: #444;
}
* { box-sizing: border-box; }
body {
  margin: 0;
  padding: 20px;
  background: var(--bg);
  color: var(--ink);
  font: 14px/1.35 "Segoe UI", Tahoma, sans-serif;
}
h1 {
  margin: 0 0 14px;
  font-size: 20px;
}
.summary {
  margin: 0 0 18px;
  color: var(--muted);
}
.table-block {
  background: var(--panel);
  border: 1px solid #d5d5d5;
  border-radius: 8px;
  padding: 12px 12px 14px;
  margin: 0 0 18px;
}
.table-title {
  margin: 0 0 8px;
  font-size: 18px;
}
.table-meta {
  margin: 0 0 10px;
  color: var(--muted);
  font-size: 12px;
}
.frame-grid {
  display: grid;
  grid-template-columns: repeat("""
        + str(columns)
        + """, minmax(132px, 1fr));
  gap: 8px;
  align-items: start;
}
.frame-cell {
  min-width: 0;
}
.frame-label {
  margin: 0 0 3px;
  font-size: 11px;
  color: var(--accent);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
.frame-label.primary {
  color: var(--ink);
}
.wave-svg {
  display: block;
  width: 100%;
  height: auto;
  background: #fafafa;
}
.wave-outline {
  fill: none;
  stroke: var(--cell-border);
  stroke-width: 1;
}
.wave-mid {
  stroke: var(--grid-line);
  stroke-width: 1;
}
.wave-path {
  fill: none;
  stroke: var(--wave);
  stroke-width: 2;
  stroke-linejoin: round;
  stroke-linecap: round;
}
@media (max-width: 900px) {
  .frame-grid {
    grid-template-columns: repeat(4, minmax(120px, 1fr));
  }
}
@media (max-width: 560px) {
  body { padding: 12px; }
  .frame-grid {
    grid-template-columns: repeat(2, minmax(120px, 1fr));
  }
}
</style>
</head>
<body>
"""
    )
    parts.append("<h1>PPG Wavetable Waveform Overview</h1>")
    parts.append(
        f'<p class="summary">Generated from {len(wavetables)} header files. '
        "Each thumbnail shows one single-cycle frame.</p>"
    )

    for wt in wavetables:
        parts.append('<section class="table-block">')
        name = wt["path"].stem
        parts.append(f'<h2 class="table-title">{html.escape(name)}</h2>')
        parts.append(
            '<p class="table-meta">'
            f'{wt["frame_count"]} frames, {wt["samples_per_frame"]} samples/frame'
            f' &middot; source: {html.escape(str(wt["path"]))}</p>'
        )
        parts.append('<div class="frame-grid">')
        for frame_idx, samples in enumerate(wt["frames"]):
            marker = wt["frame_markers"][frame_idx] if frame_idx < len(wt["frame_markers"]) else frame_idx
            label = f"Wave {frame_idx} ({marker:02X}H)"
            label_class = "frame-label primary" if frame_idx == 0 else "frame-label"
            points = waveform_svg_points(samples)
            parts.append('<div class="frame-cell">')
            parts.append(f'<p class="{label_class}">{html.escape(label)}</p>')
            parts.append(
                '<svg class="wave-svg" viewBox="0 0 132 88" preserveAspectRatio="none" '
                f'aria-label="{html.escape(label)}">'
                '<rect class="wave-outline" x="0.5" y="0.5" width="131" height="87" />'
                '<line class="wave-mid" x1="1" y1="44" x2="131" y2="44" />'
                f'<polyline class="wave-path" points="{points}" />'
                "</svg>"
            )
            parts.append("</div>")
        parts.append("</div>")
        parts.append("</section>")

    parts.append("</body></html>\n")
    return "\n".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Create an HTML page with waveform diagrams for all PPG wavetable frames "
            "found in a directory of PpgWavetable*.h headers."
        )
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=Path("sources/Application/Instruments/Wavetables/PPG"),
        help="Directory containing PpgWavetable*.h files",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("ppg_wavetables_overview.html"),
        help="Output HTML file path",
    )
    parser.add_argument(
        "--columns",
        type=int,
        default=8,
        help="Number of thumbnail columns per wavetable section (desktop layout)",
    )
    args = parser.parse_args()

    if args.columns < 1:
        raise SystemExit("--columns must be >= 1")
    if not args.input_dir.is_dir():
        raise SystemExit(f"Input directory not found: {args.input_dir}")

    headers = sorted(args.input_dir.glob("PpgWavetable*.h"))
    if not headers:
        raise SystemExit(f"No PpgWavetable*.h files found in {args.input_dir}")

    wavetables = [parse_wavetable_header(path) for path in headers]
    html_text = render_html(wavetables, columns=args.columns)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(html_text, encoding="utf-8")
    print(f"Wrote {args.output} ({len(wavetables)} wavetables)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
