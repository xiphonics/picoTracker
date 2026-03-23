#!/usr/bin/env python3
"""
png2font_c.py

Usage:
    python3 png2font_c.py font.png [--threshold 128]

Reads a 160x160 PNG (16x16 glyphs of 10x10 pixels each), starting at ASCII 32,
and writes a header file <basename>.h with:

static const uint16_t FONT_WIDE_BITMAP[224][10] = { ... };

Left pixel maps to bit 9, right pixel to bit 0 (lower 10 bits used).
"""
import sys
import os
import argparse
from PIL import Image
import re

GLYPH_COLS = 16
GLYPH_ROWS = 16
GLYPH_W = 10
GLYPH_H = 10

def sanitize_ident(name: str) -> str:
    # convert basename to uppercase C identifier
    s = re.sub(r'[^0-9a-zA-Z_]', '_', name)
    # if it starts with a digit, prepend underscore
    if re.match(r'^\d', s):
        s = '_' + s
    return s.upper()

def glyph_row_to_uint16(row_pixels, threshold):
    """
    row_pixels: iterable of length GLYPH_W, each a grayscale value 0..255
    returns 16-bit int, left pixel is bit 9, right pixel bit 0
    """
    val = 0
    for x, px in enumerate(row_pixels):
        bit_index = x  # leftmost -> 9, rightmost -> 0
        if px <= threshold:
            val |= (1 << bit_index)
    return val

def process_image(img_path, threshold, start, end):
    GLYPH_COUNT = end - start + 1
    im = Image.open(img_path)
    im = im.convert('L')  # grayscale
    w, h = im.size
    expected_w = GLYPH_COLS * GLYPH_W
    expected_h = GLYPH_ROWS * GLYPH_H
    if (w, h) != (expected_w, expected_h):
        raise SystemExit(f"ERROR: image size must be {expected_w}x{expected_h}. Got {w}x{h}")
    pixels = im.load()

    glyphs = []  # list of glyphs, each glyph is list of 10 uint16 rows
    for gy in range(GLYPH_ROWS):
        for gx in range(GLYPH_COLS):
            glyph_rows = []
            left = gx * GLYPH_W
            top = gy * GLYPH_H
            for row in range(GLYPH_H):
                row_pixels = [pixels[left + col, top + row] for col in range(GLYPH_W)]
                row_val = glyph_row_to_uint16(row_pixels, threshold)
                glyph_rows.append(row_val)
            glyphs.append(glyph_rows)

    # glyphs length should be 256; we need only 224 starting at index corresponding to FIRST_CODE.
    # In grid order glyph index 0 => char 0 (but we treat 0..255 mapping).
    # We want characters 32..255 which correspond to glyph indices 32..255.
    selected = glyphs[start:end+1]
    if len(selected) != GLYPH_COUNT:
        raise RuntimeError("internal error: unexpected glyph selection count")
    return selected

def write_header(basename, glyphs, out_path, start):
    ident_base = sanitize_ident(basename)
    guard = f"_{ident_base}_H_"
    
    with open(out_path, 'w', newline='\n') as f:
        f.write(f"#ifndef _FONT_{basename.upper()}_H\n")
        f.write(f"#define _FONT_{basename.upper()}_H\n\n")
        f.write(f"#include <stdint.h>\n\n")
        f.write(f"/* 8x8 wide font by nILS, given to the public domain */\n\n")
        f.write(f"static const uint16_t FONT_{basename.upper()}_BITMAP[{len(glyphs)}][{GLYPH_H}] = {{\n")

        code = start
        for g in glyphs:
            # comment with code and printable char where meaningful
            if 32 <= code <= 127:
                char_repr = chr(code)
                if code == 92:  # backslash
                    char_repr = 'backslash'
                comment = f" // {char_repr}"
            else:
                comment = f" // {code}"
            row_hex = ", ".join(f"0x{val:03X}" for val in g)
            f.write(f"    {{{row_hex}}},{comment}\n")
            code += 1

        f.write("};\n\n")
        f.write(f"#endif // FONT_{basename.upper()}_H\n\n")

def main():
    p = argparse.ArgumentParser(description="Convert 16x16 grid PNG (10x10 glyphs) -> C header array")
    p.add_argument("png", help="input PNG filename (expected 160x160)")
    p.add_argument("--threshold", "-t", type=int, default=128, help="grayscale threshold (0-255) for 'on' pixels")
    p.add_argument("--out", "-o", help="output header filename (default: <basename>.h)")
    p.add_argument("--start", "-s", help="start index to map from (default: 32)", type=int, default=32)
    p.add_argument("--end", "-e", help="end index to map to (default: 127)", type=int, default=127)
    p.add_argument("--name", "-n", help="font name to be used", type=str, default="wide")
    args = p.parse_args()

    png = args.png
    if not os.path.exists(png):
        raise SystemExit(f"Input file not found: {png}")

    base = os.path.splitext(os.path.basename(png))[0]
    out = args.out if args.out else "font_" + args.name.lower() + ".h"

    glyphs = process_image(png, args.threshold, args.start, args.end)
    write_header(args.name, glyphs, out, args.start)
    print(f"Wrote {out} ({len(glyphs)} glyphs, {GLYPH_W}x{GLYPH_H} each)")

if __name__ == "__main__":
    main()