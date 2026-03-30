#!/usr/bin/env python3
"""Convert an image to an LVGL-compatible C array for monochrome displays."""

import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Install Pillow first: pip3 install Pillow")
    sys.exit(1)


def convert(input_path: str, output_path: str, threshold: int = 128):
    img = Image.open(input_path).convert("L")  # Convert to grayscale
    width, height = img.size
    print(f"Image size: {width}x{height}")

    pixels = img.load()

    # LVGL expects rows padded to full bytes
    row_bytes = (width + 7) // 8

    # Build palette: 2 entries, each is lv_color32_t (B, G, R, A) = 4 bytes
    # Palette[0] = white (for bit 0)
    # Palette[1] = black (for bit 1)
    palette = [
        0xFF, 0xFF, 0xFF, 0xFF,  # Index 0 = white (B=0xFF, G=0xFF, R=0xFF, A=0xFF)
        0x00, 0x00, 0x00, 0xFF,  # Index 1 = black (B=0x00, G=0x00, R=0x00, A=0xFF)
    ]

    pixel_data = []
    for y in range(height):
        for bx in range(row_bytes):
            byte = 0
            for bit in range(8):
                x = bx * 8 + bit
                if x < width:
                    # Pixel darker than threshold = black = index 1 (bit set)
                    if pixels[x, y] < threshold:
                        byte |= 1 << (7 - bit)
            pixel_data.append(byte)

    data = palette + pixel_data

    # Generate C file
    var_name = Path(output_path).stem
    total_bytes = len(data)

    lines = []
    lines.append("#include <lvgl.h>")
    lines.append("")
    lines.append("#ifndef LV_ATTRIBUTE_MEM_ALIGN")
    lines.append("#define LV_ATTRIBUTE_MEM_ALIGN")
    lines.append("#endif")
    lines.append("")
    lines.append(f"static LV_ATTRIBUTE_MEM_ALIGN const uint8_t {var_name}_map[] = {{")

    # Write data in rows of 12 bytes
    for i in range(0, total_bytes, 12):
        chunk = data[i : i + 12]
        hex_vals = ", ".join(f"0x{b:02x}" for b in chunk)
        lines.append(f"    {hex_vals},")

    lines.append("};")
    lines.append("")
    lines.append(f"const lv_img_dsc_t {var_name} = {{")
    lines.append("    .header = {")
    lines.append(f"        .cf = LV_IMG_CF_INDEXED_1BIT,")
    lines.append(f"        .w = {width},")
    lines.append(f"        .h = {height},")
    lines.append("    },")
    lines.append(f"    .data_size = {total_bytes},")
    lines.append(f"    .data = {var_name}_map,")
    lines.append("};")
    lines.append("")

    with open(output_path, "w") as f:
        f.write("\n".join(lines))

    print(f"Written to {output_path}")
    print(f"  {total_bytes} bytes ({len(palette)} palette + {len(pixel_data)} pixels)")
    print(f"  Format: LV_IMG_CF_INDEXED_1BIT")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input_image> [output.c] [threshold 0-255]")
        print(f"  threshold: pixels darker than this become black (default: 128)")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else str(Path(input_file).with_suffix(".c"))
    threshold = int(sys.argv[3]) if len(sys.argv) > 3 else 128

    convert(input_file, output_file, threshold)
