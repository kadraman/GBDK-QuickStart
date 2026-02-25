#!/usr/bin/env python3
"""
gen_background.py
=================
Generates the scrollable background for the GBC-Template project.

Outputs
-------
  res/background.png      – 256x144 indexed PNG (32x18 tiles of 8x8 pixels)
  res/background.c        – tile data, palettes, tilemap and attr_map
  res/background.h        – header with BACKGROUND_MAP_WIDTH=32

The map is 32 tiles wide (256 px) so the game can demonstrate horizontal
scrolling.  The visible GBC screen is 160 px (20 tiles); the extra 12 tiles
give 96 pixels of smooth scroll range.

Palette layout
--------------
  Palette 0 (sky/clouds/tree-foliage, rows 0-9):
    0 = sky blue  (155,200,234)
    1 = white     (255,255,255)   clouds
    2 = tree green(60,150,60)     foliage
    3 = tree dark (40,80,20)      foliage shadow / tree trunk

  Palette 1 (ground/grass, rows 10-17):
    0 = grass light(120,200,80)
    1 = grass mid  (80,160,80)
    2 = dirt brown (120,80,40)
    3 = dirt dark  (80,50,20)

attr_map
--------
  For each of the 32×18 = 576 tile positions, one byte:
    0x00 = use palette 0  (rows 0-9)
    0x01 = use palette 1  (rows 10-17)
  Loaded into VRAM bank 1 via:
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_attr_map);
    VBK_REG = 0;

Tile index list (15 tiles)
--------------------------
  0  Sky solid
  1  Cloud top-left
  2  Cloud top-centre
  3  Cloud top-right
  4  Cloud bottom-left
  5  Cloud bottom-centre
  6  Cloud bottom-right
  7  Tree foliage top-left
  8  Tree foliage top-right
  9  Tree foliage bottom-left
 10  Tree foliage bottom-right
 11  Tree trunk (2-pixel wide, sky bg → blends into both palettes)
 12  Grass top  (lighter highlight on top row)
 13  Dirt / sub-soil
 14  Deep ground

Reuse: edit TILES / TILEMAP_FLAT / PALETTE_COLORS then run:
  python3 tools/gen_background.py
"""

import os, sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT  = os.path.dirname(TOOLS_DIR)
sys.path.insert(0, TOOLS_DIR)

from gbc_asset_builder import make_indexed_png, write_background_files

# ---------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    # Palette 0: sky / clouds / tree foliage
    (155, 200, 234),  # 0 sky blue
    (255, 255, 255),  # 1 cloud white
    ( 60, 150,  60),  # 2 tree green
    ( 40,  80,  20),  # 3 tree dark / trunk
    # Palette 1: ground / grass / dirt
    (120, 200,  80),  # 0 grass light
    ( 80, 160,  80),  # 1 grass mid
    (120,  80,  40),  # 2 dirt brown
    ( 80,  50,  20),  # 3 dirt dark
]

# PNG preview palette – 4 colours that represent the tile designs
PNG_PALETTE = [
    (155, 200, 234),  # 0 sky blue
    (255, 255, 255),  # 1 white / cloud
    ( 60, 150,  60),  # 2 green (tree foliage / grass, context-dependent)
    ( 40,  80,  20),  # 3 dark  (tree dark / dirt dark, context-dependent)
]

S = 0  # sky
W = 1  # white / cloud
G = 2  # green (foliage / grass)
D = 3  # dark  (tree outline / dirt)

# ---------------------------------------------------------------------------
# 8x8 tile definitions
# ---------------------------------------------------------------------------
TILES = [
    # 0: Sky solid
    [[S]*8]*8,

    # 1: Cloud top-left  (lower-left fills with W)
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W]],

    # 2: Cloud top-centre (top 2 rows blank)
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W]],

    # 3: Cloud top-right
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S]],

    # 4: Cloud bottom-left
    [[S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 5: Cloud bottom-centre
    [[W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [W,W,W,W,W,W,W,W],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 6: Cloud bottom-right
    [[W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [W,W,W,W,W,W,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 7: Tree foliage top-left (rounded corner)
    [[S,S,S,D,G,G,G,G],
     [S,S,D,G,G,G,G,G],
     [S,D,G,G,G,G,G,G],
     [D,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G]],

    # 8: Tree foliage top-right
    [[G,G,G,G,D,S,S,S],
     [G,G,G,G,G,D,S,S],
     [G,G,G,G,G,G,D,S],
     [G,G,G,G,G,G,G,D],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G]],

    # 9: Tree foliage bottom-left
    [[G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [D,G,G,G,G,G,G,G],
     [D,G,G,G,G,G,G,G],
     [S,D,G,G,G,G,G,G],
     [S,S,D,D,G,G,G,G],
     [S,S,S,D,D,D,G,G]],

    # 10: Tree foliage bottom-right
    [[G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,D],
     [G,G,G,G,G,G,G,D],
     [G,G,G,G,G,G,D,S],
     [G,G,G,G,D,D,S,S],
     [G,G,D,D,D,S,S,S]],

    # 11: Tree trunk (2-pixel wide dark strip, sky background)
    [[S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S]],

    # 12: Grass top (lighter highlight on row 0)
    [[G,G,G,G,G,G,G,G],   # colour 2 = grass light in palette 1
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G]],

    # 13: Dirt (brown with some dark spots)
    [[D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,S,D,D,D,S,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,S,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [S,D,D,D,D,D,S,D],
     [D,D,D,D,D,D,D,D]],

    # 14: Deep ground (solid dark)
    [[D]*8]*8,
]
assert len(TILES) == 15, f"Expected 15 tiles, got {len(TILES)}"

# ---------------------------------------------------------------------------
# 32x18 tilemap
# ---------------------------------------------------------------------------
_SKY    = 0
_CTL,_CTC,_CTR = 1,2,3   # cloud top
_CBL,_CBC,_CBR = 4,5,6   # cloud bottom
_FTL,_FTR = 7,8           # foliage top
_FBL,_FBR = 9,10           # foliage bottom
_TRK    = 11               # trunk
_GRASS  = 12
_DIRT   = 13
_DEEP   = 14

MAP_W, MAP_H = 32, 18

# Trees: each entry is (left_col, right_col)
_TREES = [(6,7), (16,17), (24,25), (29,30)]

def _build_tilemap():
    rows = []
    for row in range(MAP_H):
        r = []
        for col in range(MAP_W):
            tile = _SKY

            if row >= 14:
                tile = _DEEP
            elif row >= 11:
                tile = _DIRT
            elif row == 10:
                tile = _GRASS
            # Cloud 1: rows 2-3, cols 3-5
            elif row == 2 and 3 <= col <= 5:
                tile = [_CTL,_CTC,_CTR][col-3]
            elif row == 3 and 3 <= col <= 5:
                tile = [_CBL,_CBC,_CBR][col-3]
            # Cloud 2: rows 1-2, cols 22-24
            elif row == 1 and 22 <= col <= 24:
                tile = [_CTL,_CTC,_CTR][col-22]
            elif row == 2 and 22 <= col <= 24:
                tile = [_CBL,_CBC,_CBR][col-22]
            else:
                # Trees
                for lc, rc in _TREES:
                    if row == 7 and col == lc:   tile = _FTL; break
                    if row == 7 and col == rc:   tile = _FTR; break
                    if row == 8 and col == lc:   tile = _FBL; break
                    if row == 8 and col == rc:   tile = _FBR; break
                    if row == 9 and col in (lc, rc): tile = _TRK; break

            r.append(tile)
        rows.append(r)
    return [t for row in rows for t in row]

TILEMAP_FLAT = _build_tilemap()
assert len(TILEMAP_FLAT) == MAP_W * MAP_H

# ---------------------------------------------------------------------------
# Per-tile palette attribute map
# ---------------------------------------------------------------------------
ATTR_MAP = [0x00 if (i // MAP_W) < 10 else 0x01
            for i in range(MAP_W * MAP_H)]

# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------
def main():
    out_dir = os.path.join(REPO_ROOT, 'res')
    os.makedirs(out_dir, exist_ok=True)

    # Build 256×144 PNG from tilemap + tiles
    pixel_grid = [[S] * (MAP_W * 8) for _ in range(MAP_H * 8)]
    for idx, tile_id in enumerate(TILEMAP_FLAT):
        row, col = divmod(idx, MAP_W)
        tile = TILES[tile_id]
        for ty in range(8):
            for tx in range(8):
                pixel_grid[row*8+ty][col*8+tx] = tile[ty][tx]

    png_path = os.path.join(out_dir, 'background.png')
    make_indexed_png(pixel_grid, PNG_PALETTE, png_path)
    print(f'Written {png_path}  ({MAP_W*8}x{MAP_H*8})')

    write_background_files(
        name='background',
        tiles=TILES,
        tilemap=TILEMAP_FLAT,
        palette_colors=PALETTE_COLORS,
        map_width=MAP_W,
        map_height=MAP_H,
        out_dir=out_dir,
        attr_map=ATTR_MAP,
    )

if __name__ == '__main__':
    main()
