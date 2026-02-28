#!/usr/bin/env python3
"""
gen_background.py
=================
Auto-discovers background definitions in res/backgrounds/*/definition.py and
generates .c/.h source files plus a preview PNG for each background.

Usage
-----
  python3 tools/gen_background.py                                      # process all
  python3 tools/gen_background.py res/backgrounds/gameplay/definition.py  # one

Adding a new background
-----------------------
1. Create a directory under res/backgrounds/, e.g. res/backgrounds/myscreen/
2. Add a definition.py with the following module-level names:
     NAME          – str, output file base name (e.g. 'bg_myscreen')
     TILES         – list of 8x8 pixel tile grids (list of 8 lists of 8 ints)
     TILEMAP_FLAT  – flat list of tile indices (MAP_W * MAP_H)
     PALETTE_COLORS– (r,g,b) tuples, length == n_palettes * 4
     PNG_PALETTE   – (r,g,b) tuples for the PNG preview
     MAP_W, MAP_H  – tilemap dimensions in tiles
     ATTR_MAP      – flat list of per-tile palette attribute bytes
3. Run  make generate  (or  python3 tools/gen_background.py)

Output per background
---------------------
  res/<name>.png        – preview PNG
  res/<name>.c          – GBDK tile data, palettes, tilemap, attr_map
  res/<name>.h          – header with tile/map constants

Requirements:  pip install pillow
"""

import importlib.util
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(TOOLS_DIR)
sys.path.insert(0, TOOLS_DIR)

from gbc_asset_builder import make_indexed_png, write_background_files


def _load_definition(path):
    """Import a definition.py file as a Python module."""
    spec = importlib.util.spec_from_file_location('background_definition', path)
    mod  = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def process_definition(defn_path):
    """Generate .png, .c, and .h for one background definition file."""
    mod = _load_definition(defn_path)

    name               = mod.NAME
    tiles              = mod.TILES
    tilemap_flat       = mod.TILEMAP_FLAT
    palette_colors     = mod.PALETTE_COLORS
    png_palette        = mod.PNG_PALETTE
    map_w              = mod.MAP_W
    map_h              = mod.MAP_H
    attr_map           = mod.ATTR_MAP
    collision_tile_ids = getattr(mod, 'COLLISION_TILE_IDS', None)
    solid_tile_ids     = getattr(mod, 'SOLID_TILE_IDS',     None)

    out_dir = os.path.join(REPO_ROOT, 'res')
    os.makedirs(out_dir, exist_ok=True)

    # Build preview PNG from tilemap + tiles
    pixel_grid = [[0] * (map_w * 8) for _ in range(map_h * 8)]
    for idx, tile_id in enumerate(tilemap_flat):
        row, col = divmod(idx, map_w)
        tile = tiles[tile_id]
        for ty in range(8):
            for tx in range(8):
                pixel_grid[row * 8 + ty][col * 8 + tx] = tile[ty][tx]

    png_path = os.path.join(out_dir, f'{name}.png')
    make_indexed_png(pixel_grid, png_palette, png_path)
    print(f'Written {png_path}  ({map_w * 8}x{map_h * 8})')

    write_background_files(
        name=name,
        tiles=tiles,
        tilemap=tilemap_flat,
        palette_colors=palette_colors,
        map_width=map_w,
        map_height=map_h,
        out_dir=out_dir,
        attr_map=attr_map,
        collision_tile_ids=collision_tile_ids,
        solid_tile_ids=solid_tile_ids,
        generator='gen_background.py',
    )


def main():
    # Allow overriding from command line: python3 gen_background.py path/to/def.py
    if len(sys.argv) > 1:
        for defn_path in sys.argv[1:]:
            print(f'=== Processing {defn_path} ===')
            process_definition(os.path.abspath(defn_path))
        return

    backgrounds_dir = os.path.join(REPO_ROOT, 'res', 'backgrounds')
    if not os.path.isdir(backgrounds_dir):
        print(f'No backgrounds directory at {backgrounds_dir} – nothing to do.')
        return

    found = 0
    for entry in sorted(os.listdir(backgrounds_dir)):
        defn_path = os.path.join(backgrounds_dir, entry, 'definition.py')
        if os.path.isfile(defn_path):
            print(f'=== Processing background: {entry} ===')
            process_definition(defn_path)
            found += 1

    if found == 0:
        print('No background definitions found in res/backgrounds/')
    else:
        print(f'\nProcessed {found} background(s).')


if __name__ == '__main__':
    main()
