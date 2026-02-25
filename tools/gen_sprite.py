#!/usr/bin/env python3
"""
gen_sprite.py
=============
Auto-discovers sprite definitions in res/sprites/*/definition.py and
generates .c/.h source files plus a preview PNG for each sprite.

Usage
-----
  python3 tools/gen_sprite.py                         # process all sprites
  python3 tools/gen_sprite.py res/sprites/player/definition.py  # one sprite

Adding a new sprite
-------------------
1. Create a directory under res/sprites/, e.g. res/sprites/enemy/
2. Add a definition.py with the following module-level names:
     NAME         – str, base symbol name (e.g. 'enemy')
     PALETTE      – list of 4 (R,G,B) tuples; index 0 = transparent
     PIXEL_CHARS  – dict mapping char -> colour index; '.' is always 0
     ANIMATIONS   – dict { anim_name: [(top_rows, bot_rows), ...] }
                    top_rows / bot_rows: list of 8 strings of 8 chars
3. Run  make generate  (or  python3 tools/gen_sprite.py)

Output
------
  res/<name>.png   – preview PNG (8 px wide × N*16 px tall, all frames)
  res/<name>.c     – GBDK 2bpp tile data + palette
  res/<name>.h     – header with PLAYER_ANIM_<ANIM>_START / _FRAMES defines

Requirements:  pip install pillow
"""

import importlib.util
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT  = os.path.dirname(TOOLS_DIR)
sys.path.insert(0, TOOLS_DIR)

from gbc_asset_builder import make_indexed_png, write_sprite_files_animated


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _load_definition(path):
    """Import a definition.py file as a Python module."""
    spec = importlib.util.spec_from_file_location('sprite_definition', path)
    mod  = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _parse_tile(string_rows, pixel_chars):
    tile = []
    for row_str in string_rows:
        row = [pixel_chars.get(c, 0) for c in row_str]
        assert len(row) == 8, \
            f"Row '{row_str}' must be exactly 8 chars (got {len(row)})"
        tile.append(row)
    assert len(tile) == 8, \
        f"Tile half must have exactly 8 rows (got {len(tile)})"
    return tile


def process_definition(defn_path):
    """Generate .png, .c, and .h for one sprite definition file."""
    mod = _load_definition(defn_path)

    name        = mod.NAME
    palette     = mod.PALETTE
    pixel_chars = dict(getattr(mod, 'PIXEL_CHARS', {}))
    pixel_chars['.'] = 0   # '.' is always transparent

    animations = mod.ANIMATIONS

    out_dir = os.path.join(REPO_ROOT, 'res')
    os.makedirs(out_dir, exist_ok=True)

    # Build a preview PNG: all frames stacked vertically (8 px wide)
    pixel_grid = []
    for anim_frames in animations.values():
        for top_rows, bot_rows in anim_frames:
            for row_str in top_rows:
                pixel_grid.append([pixel_chars.get(c, 0) for c in row_str])
            for row_str in bot_rows:
                pixel_grid.append([pixel_chars.get(c, 0) for c in row_str])

    n_total_frames = sum(len(v) for v in animations.values())
    png_path = os.path.join(out_dir, f'{name}.png')
    make_indexed_png(pixel_grid, palette, png_path)
    print(f'Written {png_path}  (8x{n_total_frames * 16})')

    write_sprite_files_animated(
        name=name,
        animations=animations,
        palette_colors=palette,
        pixel_chars=pixel_chars,
        out_dir=out_dir,
    )


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    # Allow overriding from command line: python3 gen_sprite.py path/to/def.py
    if len(sys.argv) > 1:
        for defn_path in sys.argv[1:]:
            print(f'=== Processing {defn_path} ===')
            process_definition(os.path.abspath(defn_path))
        return

    sprites_dir = os.path.join(REPO_ROOT, 'res', 'sprites')
    if not os.path.isdir(sprites_dir):
        print(f'No sprites directory at {sprites_dir} – nothing to do.')
        return

    found = 0
    for entry in sorted(os.listdir(sprites_dir)):
        defn_path = os.path.join(sprites_dir, entry, 'definition.py')
        if os.path.isfile(defn_path):
            print(f'=== Processing sprite: {entry} ===')
            process_definition(defn_path)
            found += 1

    if found == 0:
        print('No sprite definitions found in res/sprites/')
    else:
        print(f'\nProcessed {found} sprite(s).')


if __name__ == '__main__':
    main()
