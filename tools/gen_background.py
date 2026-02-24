#!/usr/bin/env python3
"""
gen_background.py
=================
Generates the background tileset for the GBC-Template project.

Outputs
-------
  res/background.png   – 160x144 indexed PNG (20x18 tiles of 8x8 pixels)
  res/background.c     – GBDK tile data, palette, and tilemap
  res/background.h     – Corresponding header file

The background shows a sky-and-clouds scene in the upper portion and a
grass-and-ground scene in the lower portion.  Two GBC palettes are used:
  Palette 0 – sky/cloud colours (loaded for the upper half of the map)
  Palette 1 – ground colours   (loaded for the lower half of the map)

Tile layout (colour indices per palette)
-----------------------------------------
  0  Sky solid          – colour 0 (sky blue) everywhere
  1  Cloud left edge    – lower-left corner filled with colour 1 (white)
  2  Cloud centre top   – top 2 rows blank, rest colour 1
  3  Cloud right edge   – lower-right corner filled with colour 1
  4  Cloud left lower   – top rows colour 1, rest blank
  5  Cloud centre       – top 6 rows colour 1, rest blank
  6  Cloud right lower  – top rows colour 1 left 6 cols, rest blank
  7  Cloud 2 left edge  – lower-right 5 cols colour 1
  8  Cloud 2 centre     – lower 5 rows colour 1
  9  Cloud 2 right edge – lower-left 5 cols colour 1
 10  Cloud 3 left       – top 3 rows colour 1 in right 5 cols
 11  Cloud 3 centre     – top 5 rows colour 1
 12  Cloud 3 right      – top 3 rows colour 1 in left 5 cols
 13  Grass solid        – colour 2 (green) everywhere
 14  Ground solid       – colour 3 (brown) everywhere

Tilemap (20 columns x 18 rows)
-------------------------------
  Rows 0-1   – all sky (tile 0)
  Row 2      – cloud 1 at cols 2-4 (tiles 1,2,3); sky elsewhere
  Row 3      – cloud 1 lower at cols 2-4 (tiles 4,5,6); sky elsewhere
  Row 4      – cloud 2 at cols 13-15 (tiles 7,8,9); sky elsewhere
  Row 5      – cloud 2 lower at cols 13-15 (tiles 10,11,12); sky elsewhere
  Rows 6-9   – all sky
  Row 10     – alternating grass/sky (tiles 13 and 0, 10 of each)
  Rows 11-13 – all grass (tile 13)
  Rows 14-17 – all ground (tile 14)

Reuse
-----
To create a different background, edit TILES, TILEMAP, and PALETTE_COLORS
below, then run:  python3 tools/gen_background.py
"""

import os
import sys

# Allow running from any working directory
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(TOOLS_DIR)
sys.path.insert(0, TOOLS_DIR)

from gbc_asset_builder import make_indexed_png, png_to_tiles, write_background_files

# ---------------------------------------------------------------------------
# Palette colours – 4 entries per palette, 2 palettes = 8 (r,g,b) tuples
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    # Palette 0: sky / cloud colours
    (155, 200, 234),  # index 0 – sky blue
    (255, 255, 255),  # index 1 – cloud white
    ( 52, 136,  52),  # index 2 – grass green (unused in sky palette but kept for VRAM symmetry)
    (120,  80,  40),  # index 3 – ground brown (unused in sky palette)
    # Palette 1: ground colours
    ( 52, 136,  52),  # index 0 – grass green
    ( 80, 160,  80),  # index 1 – lighter grass
    (120,  80,  40),  # index 2 – ground brown
    ( 80,  50,  20),  # index 3 – darker ground
]

# PNG palette (first 4 colours are used for all tile designs below)
# We use the same 4 colour roles when drawing tiles.
S = 0   # Sky  / grass (context-dependent)
W = 1   # White (cloud)
G = 2   # Grass green / lighter grass
D = 3   # Dark brown / ground

# ---------------------------------------------------------------------------
# 8x8 tile definitions – each tile is 8 rows of 8 colour-index pixels
# ---------------------------------------------------------------------------
TILES = [
    # ---- Tile 0: sky solid ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 1: cloud-1 left edge (lower-left corner is cloud) ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W]],

    # ---- Tile 2: cloud-1 centre top ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W]],

    # ---- Tile 3: cloud-1 right edge (lower-right corner is cloud) ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S]],

    # ---- Tile 4: cloud-1 lower-left ----
    [[S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 5: cloud-1 lower centre ----
    [[W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 6: cloud-1 lower-right ----
    [[W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 7: cloud-2 left edge ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,W,W,W,W,W],
     [S,S,S,W,W,W,W,W],
     [S,S,S,W,W,W,W,W]],

    # ---- Tile 8: cloud-2 centre ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W]],

    # ---- Tile 9: cloud-2 right edge ----
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [W,W,W,W,W,S,S,S],
     [W,W,W,W,W,S,S,S],
     [W,W,W,W,W,S,S,S]],

    # ---- Tile 10: cloud-2 lower-left ----
    [[S,S,S,W,W,W,W,W],
     [S,S,S,W,W,W,W,W],
     [S,S,S,W,W,W,W,W],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 11: cloud-2 lower centre ----
    [[W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 12: cloud-2 lower-right ----
    [[W,W,W,W,W,S,S,S],
     [W,W,W,W,W,S,S,S],
     [W,W,W,W,W,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # ---- Tile 13: grass solid (all colour 2 = green) ----
    [[G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G]],

    # ---- Tile 14: ground solid (all colour 3 = brown) ----
    [[D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D]],
]

# ---------------------------------------------------------------------------
# 20x18 tilemap – indices into TILES[]
# ---------------------------------------------------------------------------
_SKY   = 0
_GRASS = 13
_GND   = 14

TILEMAP = (
    # Row 0-1: all sky
    [_SKY] * 20,
    [_SKY] * 20,
    # Row 2: cloud 1 upper at cols 2-4
    [_SKY, _SKY, 1, 2, 3, _SKY, _SKY, _SKY, _SKY, _SKY,
     _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY],
    # Row 3: cloud 1 lower at cols 2-4
    [_SKY, _SKY, 4, 5, 6, _SKY, _SKY, _SKY, _SKY, _SKY,
     _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY],
    # Row 4: cloud 2 upper at cols 13-15
    [_SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY,
     _SKY, _SKY, _SKY, 7, 8, 9, _SKY, _SKY, _SKY, _SKY],
    # Row 5: cloud 2 lower at cols 13-15
    [_SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY, _SKY,
     _SKY, _SKY, _SKY, 10, 11, 12, _SKY, _SKY, _SKY, _SKY],
    # Rows 6-9: all sky
    [_SKY] * 20,
    [_SKY] * 20,
    [_SKY] * 20,
    [_SKY] * 20,
    # Row 10: alternating grass/sky
    [_GRASS if i % 2 == 0 else _SKY for i in range(20)],
    # Rows 11-13: all grass
    [_GRASS] * 20,
    [_GRASS] * 20,
    [_GRASS] * 20,
    # Rows 14-17: all ground
    [_GND] * 20,
    [_GND] * 20,
    [_GND] * 20,
    [_GND] * 20,
)

# Flatten tilemap
TILEMAP_FLAT = [tile for row in TILEMAP for tile in row]

# ---------------------------------------------------------------------------
# PNG palette (RGB values matching the colour indices above)
# ---------------------------------------------------------------------------
PNG_PALETTE = [
    (155, 200, 234),   # 0 – sky blue
    (255, 255, 255),   # 1 – cloud white
    ( 52, 136,  52),   # 2 – grass green
    (120,  80,  40),   # 3 – ground brown
]


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------
def main():
    out_dir = os.path.join(REPO_ROOT, 'res')
    os.makedirs(out_dir, exist_ok=True)

    # Build the 160x144 pixel grid from tiles + tilemap
    pixel_grid = [[0] * 160 for _ in range(144)]
    map_width, map_height = 20, 18
    for row in range(map_height):
        for col in range(map_width):
            tile_idx = TILEMAP_FLAT[row * map_width + col]
            tile = TILES[tile_idx]
            for ty in range(8):
                for tx in range(8):
                    pixel_grid[row * 8 + ty][col * 8 + tx] = tile[ty][tx]

    png_path = os.path.join(out_dir, 'background.png')
    make_indexed_png(pixel_grid, PNG_PALETTE, png_path)
    print(f'Written {png_path}')

    write_background_files(
        name='background',
        tiles=TILES,
        tilemap=TILEMAP_FLAT,
        palette_colors=PALETTE_COLORS,
        map_width=map_width,
        map_height=map_height,
        out_dir=out_dir,
    )


if __name__ == '__main__':
    main()
