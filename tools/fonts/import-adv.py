from PIL import Image

# === configuration ===
INPUT_PNG = "font_adv.png"
OUTPUT_HEADER = "font_adv.h"
CHAR_WIDTH = 22
CHAR_HEIGHT = 30
GRID_COLS = 16
GRID_ROWS = 16
START_CHAR = 32
END_CHAR = 255
# ======================

def main():
    img = Image.open(INPUT_PNG).convert("L")  # grayscale
    pixels = img.load()
    w, h = img.size

    expected_w = GRID_COLS * CHAR_WIDTH
    expected_h = GRID_ROWS * CHAR_HEIGHT
    if (w, h) != (expected_w, expected_h):
        print(f"Warning: image size {w}x{h} does not match expected {expected_w}x{expected_h}")

    char_count = END_CHAR - START_CHAR + 1
    all_chars = []

    for index in range(START_CHAR, END_CHAR + 1):
        row = index // GRID_COLS
        col = index % GRID_COLS
        x0 = col * CHAR_WIDTH
        y0 = row * CHAR_HEIGHT

        char_data = []
        for y in range(CHAR_HEIGHT):
            row_data = []
            for x in range(CHAR_WIDTH):
                # invert: black → 255, white → 0
                value = 255 - pixels[x0 + x, y0 + y]
                row_data.append(value)
            char_data.append(row_data)
        all_chars.append(char_data)

    # Write header
    with open(OUTPUT_HEADER, "w") as f:
        f.write("// Auto-generated font header\n")
        f.write("// Each character is 22x30 pixels, grayscale alpha values 0–255\n\n")
        f.write("#pragma once\n\n")
        f.write(f"#define FONT_CHAR_WIDTH {CHAR_WIDTH}\n")
        f.write(f"#define FONT_CHAR_HEIGHT {CHAR_HEIGHT}\n")
        f.write(f"#define FONT_FIRST_CHAR {START_CHAR}\n")
        f.write(f"#define FONT_LAST_CHAR {END_CHAR}\n")
        f.write(f"#define FONT_CHAR_COUNT {char_count}\n\n")

        f.write(f"const unsigned char FONT[{char_count}][{CHAR_HEIGHT}][{CHAR_WIDTH}] = {{\n")

        for c_index, char_data in enumerate(all_chars):
            f.write(f"  // Character {START_CHAR + c_index}\n")
            f.write("  {\n")
            for y, row_data in enumerate(char_data):
                f.write("    { ")
                f.write(",".join(f"{v:3d}" for v in row_data))
                f.write(" },\n")
            f.write("  },\n")

        f.write("};\n")

    print(f"Exported {char_count} characters to {OUTPUT_HEADER}")

if __name__ == "__main__":
    main()