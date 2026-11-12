---
genpdf:
  format: book
  title: Cutter User Guide
  subtitle: Image cropping with chroma key support
  author: SRA, Inc.
  date: 2026-05-06
  font_size: 12pt
  page_numbers: true
  copyright: Copyright (c) 2024 SRA, Inc. All rights reserved.
  table:
    header_background: "#e8f1ff"
    header_color: "#1f2937"
    border_color: "#cbd5e1"
    border_width: "1px"
    stripe: true
    stripe_background: "#f8fafc"
    cell_padding: "0.45em 0.65em"
    font_size: "0.95em"
    compact: false
    align: left
    header_align: center
    cell_align: left
---

# cutter

An image-cropping utility built with Qt 6.

Mouse-based cropping in image viewers and editors may look adequate but make
precise pixel-level selection difficult. `cutter` accepts numeric coordinates,
widths, and heights so you can crop exactly the region you need.

## Usage

```text
cutter [--overwrite]
       [--clip-green | --cut=N | --cut-top=N | --cut-bottom=N | --cut-left=N | --cut-right=N]
       [GEOMETRY | --geometry=GEOMETRY]
       FILE
```

Crops `FILE` and writes the result as `<base>-<geometry>.<ext>`.

The output uses the input format if that format supports saving. Otherwise,
it saves as PNG and displays a message explaining the change.

## Options

- `--overwrite`: Overwrite the output without confirmation.
- `--over`: Short form of `--overwrite`.
- `--clip-green`: Make green background connected to the image edges transparent,
  crop to the smallest rectangle containing all remaining pixels, and center
  the result on a transparent canvas twice its size. Irregular subjects are supported.
- `--cut=N`: Remove `Npx` from each of the four edges.
- `--cut-top=N`: Remove `Npx` from the top.
- `--cut-bottom=N`: Remove `Npx` from the bottom.
- `--cut-left=N`: Remove `Npx` from the left.
- `--cut-right=N`: Remove `Npx` from the right.
- `--geometry=GEOMETRY`: Explicitly specify the crop region as an option.
- `--geo=GEOMETRY`: Short form of `--geometry=GEOMETRY`.

## Geometry Formats

- `WxH+X+Y`: Width, height, and offset from the top-left corner.
- `X1,Y1:X2,Y2`: Top-left and bottom-right coordinates.
- `X1,Y1-X2,Y2`: The same coordinate format, with `-` as the separator.

Omitting `GEOMETRY` uses the entire image.

Regions extending beyond the source image are clipped to its boundaries.

`--clip-green` is intended for images drawn over a green-filled background.
It tolerates some variation from `0x00ff00`, as in images saved through the
ChatGPT browser. Only background connected to the image edges is made transparent.
Circles, rectangles, and irregular shapes are handled alike; colors inside the
subject that cannot be reached from the outer green background are not treated as background.

If reflected green or antialiasing mixes green into the subject's edge, the tool
suppresses that green component and adjusts transparency. This reduces green
fringing and faded edges after background removal. Bright green parts of the
subject itself may also be affected by this edge correction.

The cropped image is centered on a transparent canvas twice its size.

`--clip-green` cannot be combined with `GEOMETRY` or the `--cut` options.

The `--cut` options cannot be combined with `GEOMETRY`.

`--cut` cannot be combined with `--cut-top`, `--cut-bottom`, `--cut-left`, or `--cut-right`.

## Examples

```bash
# Crop a JPG, preserving JPG output if saving in that format is supported
cutter photo.jpg

# Remove a 0x00ff00 background and crop
cutter --clip-green input.png

# Test chroma key cropping with the included sample
cutter --clip-green testdata/vu.png

# Trim all four edges equally
cutter --cut=5 input.png

# Trim only the top
cutter --cut-top=5 input.png

# Trim multiple edges individually
cutter --cut-top=5 --cut-right=10 input.png

# Crop with width, height, and offset
cutter 100x100+10+20 input.png

# Use the same region and overwrite existing output
cutter --overwrite 100x100+10+20 input.png

# Crop using two corner coordinates
cutter 10,20:110,120 input.png

# Geometry can also be passed as an option
cutter --geometry=10,20-110,120 input.png
```

## Output

On success, `cutter` prints the output filename to standard output.

If the output file already exists and `--overwrite` is not specified, it asks whether to overwrite it.

## Included Sample

`testdata/vu.png` is a sample for testing `--clip-green`. Running the command
makes the green background transparent and centers the subject on a transparent
canvas. The subject need not be rectangular: irregular shapes are cropped the
same way as long as the background is connected to the image edges.
