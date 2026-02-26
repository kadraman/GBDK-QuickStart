#!/usr/bin/env python3
"""
gen_font.py
===========
Auto-discovers font definitions in res/fonts/*/definition.py and
generates .c/.h source files plus a preview PNG for each font.

Usage
-----
  python3 tools/gen_font.py                                   # process all
  python3 tools/gen_font.py res/fonts/default/definition.py  # one font

Adding a new font
-----------------
1. Create a directory under res/fonts/, e.g. res/fonts/myfont/
2. Add a definition.py with the following module-level names:
     NAME          – str, output file base name (e.g. 'font_myfont')
     FONT_BITMAPS  – list of 101 glyph bitmaps (each: 8-byte list)
     PALETTE_COLORS– list of 4 (r,g,b) tuples
     PNG_PALETTE   – list of (r,g,b) tuples for the PNG preview
     EXTRA_DEFINES – optional list of (define_name, value, comment) tuples
3. Run  make generate  (or  python3 tools/gen_font.py)

Output per font
---------------
  res/<name>.png   – preview PNG sheet
  res/<name>.c     – GBDK tile data and palette
  res/<name>.h     – header with tile count and extra defines

Requirements:  pip install pillow
"""

import importlib.util
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(TOOLS_DIR)
sys.path.insert(0, TOOLS_DIR)

from gbc_asset_builder import make_indexed_png, write_font_files


def _load_definition(path):
    """Import a definition.py file as a Python module."""
    spec = importlib.util.spec_from_file_location('font_definition', path)
    mod  = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _bitmap_to_tile(bitmap_row_bytes):
    tile = []
    for byte_val in bitmap_row_bytes:
        row = [1 if (byte_val & (0x80 >> bit)) else 0 for bit in range(8)]
        tile.append(row)
    return tile


def process_definition(defn_path):
    """Generate .png, .c, and .h for one font definition file."""
    mod = _load_definition(defn_path)

    name           = mod.NAME
    font_bitmaps   = mod.FONT_BITMAPS
    palette_colors = mod.PALETTE_COLORS
    png_palette    = mod.PNG_PALETTE
    extra_defines  = getattr(mod, 'EXTRA_DEFINES', None)

    out_dir = os.path.join(REPO_ROOT, 'res')
    os.makedirs(out_dir, exist_ok=True)

    tiles = [_bitmap_to_tile(bm) for bm in font_bitmaps]

    # Build preview PNG: 16 chars wide × N rows tall
    sheet_cols = 16
    sheet_rows = (len(tiles) + sheet_cols - 1) // sheet_cols
    sheet_w, sheet_h = sheet_cols * 8, sheet_rows * 8
    pixel_grid = [[0] * sheet_w for _ in range(sheet_h)]
    for idx, tile in enumerate(tiles):
        tx = (idx % sheet_cols) * 8
        ty = (idx // sheet_cols) * 8
        for r in range(8):
            for c in range(8):
                pixel_grid[ty + r][tx + c] = tile[r][c]

    png_path = os.path.join(out_dir, f'{name}.png')
    make_indexed_png(pixel_grid, png_palette, png_path)
    print(f'Written {png_path}  ({sheet_w}x{sheet_h})')

    write_font_files(
        name=name,
        tiles=tiles,
        palette_colors=palette_colors,
        extra_defines=extra_defines,
        out_dir=out_dir,
        generator='gen_font.py',
    )


def main():
    # Allow overriding from command line: python3 gen_font.py path/to/def.py
    if len(sys.argv) > 1:
        for defn_path in sys.argv[1:]:
            print(f'=== Processing {defn_path} ===')
            process_definition(os.path.abspath(defn_path))
        return

    fonts_dir = os.path.join(REPO_ROOT, 'res', 'fonts')
    if not os.path.isdir(fonts_dir):
        print(f'No fonts directory at {fonts_dir} – nothing to do.')
        return

    found = 0
    for entry in sorted(os.listdir(fonts_dir)):
        defn_path = os.path.join(fonts_dir, entry, 'definition.py')
        if os.path.isfile(defn_path):
            print(f'=== Processing font: {entry} ===')
            process_definition(defn_path)
            found += 1

    if found == 0:
        print('No font definitions found in res/fonts/')
    else:
        print(f'\nProcessed {found} font(s).')


if __name__ == '__main__':
    main()
