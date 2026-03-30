#!/usr/bin/env python3
"""Convert an image to an LVGL-compatible C array for nice!view (monochrome, portrait-mounted).

The image is rotated 90° CW so it displays correctly on the vertically-mounted
nice!view display (which is physically 160x68 landscape).
"""

import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Install Pillow first: pip3 install Pillow")
    sys.exit(1)


def convert(input_path: str, output_path: str, threshold: int = 128):
    img = Image.open(input_path).convert("L")  # Convert to grayscale
    print(f"Original image size: {img.size[0]}x{img.size[1]}")

    # Scale to fit available portrait space (68 wide x 92 tall)
    max_w, max_h = 68, 92
    orig_w, orig_h = img.size
    scale = min(max_w / orig_w, max_h / orig_h)
    new_w, new_h = int(orig_w * scale), int(orig_h * scale)
    img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)

    # Center on a 68x92 canvas
    canvas = Image.new("L", (max_w, max_h), 255)  # white background
    paste_x = (max_w - new_w) // 2
    paste_y = (max_h - new_h) // 2
    canvas.paste(img, (paste_x, paste_y))
    img = canvas
    print(f"Scaled to {new_w}x{new_h}, padded to {max_w}x{max_h}")

    # Rotate 90° CW for landscape display (portrait → landscape)
    img = img.transpose(Image.Transpose.ROTATE_270)
    width, height = img.size
    print(f"After rotation (90° CW): {width}x{height}")

    pixels = img.load()

    # LVGL expects rows padded to full bytes
    row_bytes = (width + 7) // 8

    # Build palette: 2 entries, each is lv_color32_t (B, G, R, A) = 4 bytes
    palette = [
        0xFF, 0xFF, 0xFF, 0xFF,  # Index 0 = white
        0x00, 0x00, 0x00, 0xFF,  # Index 1 = black
    ]

    pixel_data = []
    for y in range(height):
        for bx in range(row_bytes):
            byte = 0
            for bit in range(8):
                x = bx * 8 + bit
                if x < width:
                    if pixels[x, y] < threshold:
                        byte |= 1 << (7 - bit)
            pixel_data.append(byte)

    data = palette + pixel_data

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
    print(f"  {total_bytes} bytes, {width}x{height} pixels")
    print(f"  Format: LV_IMG_CF_INDEXED_1BIT (pre-rotated for nice!view)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input_image> [output.c] [threshold 0-255]")
        print(f"  Input should be portrait (68x144 or similar)")
        print(f"  Output will be rotated 90° CW for the nice!view display")
        print(f"  threshold: pixels darker than this become black (default: 128)")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else str(Path(input_file).with_suffix(".c"))
    threshold = int(sys.argv[3]) if len(sys.argv) > 3 else 128

    convert(input_file, output_file, threshold)
