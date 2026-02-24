#!/usr/bin/env python3
"""
gen_sprite.py
=============
Generates the animated character sprite for the GBC-Template project.

Outputs
-------
  res/sprite.png   – 8x64 indexed PNG (1 tile wide × 8 tiles tall = 4 frames of 8x16)
  res/sprite.c     – GBDK tile data and sprite palette
  res/sprite.h     – Corresponding header file

The sprite is a small character that uses 8x16 sprite mode (two 8x8 OBJ tiles
stacked vertically per frame).  Four animation frames produce a simple walk cycle:

  Frame 0 – standing, arms at sides
  Frame 1 – walk pose: left leg forward
  Frame 2 – standing, arms raised (slight variation)
  Frame 3 – walk pose: right leg forward

Each 8x16 frame occupies 2 consecutive tiles in the tile data array.
The tile index for frame N starts at:  N × SPRITE_TILES_PER_FRAME

Sprite palette colours (index 0 is transparent on GBC OBJ)
-----------------------------------------------------------
  0 – transparent magenta (255,  0, 255)
  1 – body yellow         (255, 220,  0)
  2 – outfit blue         ( 50, 100, 200)
  3 – outline dark        ( 20,  20,  20)

Reuse
-----
To define a different sprite, edit FRAMES_TOP and FRAMES_BOTTOM below using
the colour aliases  T=0 (transparent), Y=1, B=2, D=3.  Each frame is a list
of 8 rows × 8 pixels.  Then run:  python3 tools/gen_sprite.py
"""

import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(TOOLS_DIR)
sys.path.insert(0, TOOLS_DIR)

from gbc_asset_builder import make_indexed_png, write_sprite_files

# ---------------------------------------------------------------------------
# Sprite palette
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    (255,   0, 255),   # 0 – transparent
    (255, 220,   0),   # 1 – yellow (head/body)
    ( 50, 100, 200),   # 2 – blue (outfit)
    ( 20,  20,  20),   # 3 – dark outline
]

PNG_PALETTE = PALETTE_COLORS

# Colour aliases for readability
T = 0   # transparent
Y = 1   # yellow
B = 2   # blue
D = 3   # dark outline

# ---------------------------------------------------------------------------
# Frame definitions – 8x16 pixels per frame, split into top (rows 0-7) and
# bottom (rows 8-15) halves.  Each is a list of 8 rows of 8 colour indices.
# ---------------------------------------------------------------------------

# ---- Frame 0: standing neutral ----
_F0_TOP = [
    [T, T, D, D, D, D, T, T],   # head outline top
    [T, D, Y, Y, Y, Y, D, T],   # head left/right
    [T, D, Y, Y, Y, Y, D, T],   # head middle
    [T, T, D, Y, Y, D, T, T],   # head chin
    [T, T, B, B, B, B, T, T],   # body top
    [T, B, B, B, B, B, B, T],   # body wide
    [T, B, B, B, B, B, B, T],   # body wide
    [T, T, D, D, D, D, T, T],   # body bottom
]
_F0_BOT = [
    [T, T, B, T, T, B, T, T],   # legs upper
    [T, T, B, T, T, B, T, T],
    [T, T, B, T, T, B, T, T],
    [T, T, B, T, T, B, T, T],
    [T, T, D, T, T, D, T, T],   # feet
    [T, T, D, T, T, D, T, T],
    [T, T, T, T, T, T, T, T],
    [T, T, T, T, T, T, T, T],
]

# ---- Frame 1: walk – left leg forward ----
_F1_TOP = [
    [T, T, D, D, D, D, T, T],
    [T, D, Y, Y, Y, Y, D, T],
    [T, D, Y, Y, Y, Y, D, T],
    [T, T, D, Y, Y, D, T, T],
    [T, T, B, B, B, B, T, T],
    [T, B, B, B, B, B, B, T],
    [T, B, B, B, B, B, B, T],
    [T, T, D, D, D, D, T, T],
]
_F1_BOT = [
    [T, D, B, T, T, B, T, T],   # left leg slightly out
    [T, D, T, T, T, B, T, T],
    [T, D, T, T, T, B, T, T],
    [D, T, T, T, T, D, T, T],
    [D, T, T, T, T, D, T, T],
    [D, D, T, T, T, T, T, T],
    [T, T, T, T, T, T, T, T],
    [T, T, T, T, T, T, T, T],
]

# ---- Frame 2: standing, arms raised ----
_F2_TOP = [
    [T, T, D, D, D, D, T, T],
    [T, D, Y, Y, Y, Y, D, T],
    [T, D, Y, Y, Y, Y, D, T],
    [T, T, D, Y, Y, D, T, T],
    [D, T, B, B, B, B, T, D],   # arms out
    [B, B, B, B, B, B, B, B],
    [T, B, B, B, B, B, B, T],
    [T, T, D, D, D, D, T, T],
]
_F2_BOT = [
    [T, T, B, T, T, B, T, T],
    [T, T, B, T, T, B, T, T],
    [T, T, B, T, T, B, T, T],
    [T, T, B, T, T, B, T, T],
    [T, T, D, T, T, D, T, T],
    [T, T, D, T, T, D, T, T],
    [T, T, T, T, T, T, T, T],
    [T, T, T, T, T, T, T, T],
]

# ---- Frame 3: walk – right leg forward ----
_F3_TOP = [
    [T, T, D, D, D, D, T, T],
    [T, D, Y, Y, Y, Y, D, T],
    [T, D, Y, Y, Y, Y, D, T],
    [T, T, D, Y, Y, D, T, T],
    [T, T, B, B, B, B, T, T],
    [T, B, B, B, B, B, B, T],
    [T, B, B, B, B, B, B, T],
    [T, T, D, D, D, D, T, T],
]
_F3_BOT = [
    [T, T, B, T, T, B, D, T],   # right leg slightly out
    [T, T, B, T, T, T, D, T],
    [T, T, B, T, T, T, D, T],
    [T, T, D, T, T, T, T, D],
    [T, T, D, T, T, T, T, D],
    [T, T, T, T, T, T, D, D],
    [T, T, T, T, T, T, T, T],
    [T, T, T, T, T, T, T, T],
]

FRAMES_TOP    = [_F0_TOP, _F1_TOP, _F2_TOP, _F3_TOP]
FRAMES_BOTTOM = [_F0_BOT, _F1_BOT, _F2_BOT, _F3_BOT]


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------
def main():
    out_dir = os.path.join(REPO_ROOT, 'res')
    os.makedirs(out_dir, exist_ok=True)

    # Build 8x64 PNG: frames stacked vertically (top half then bottom half)
    frame_count = len(FRAMES_TOP)
    png_h = frame_count * 16   # 4 frames × 16 rows = 64
    png_w = 8
    pixel_grid = []
    for top, bot in zip(FRAMES_TOP, FRAMES_BOTTOM):
        pixel_grid.extend(top)
        pixel_grid.extend(bot)

    png_path = os.path.join(out_dir, 'sprite.png')
    make_indexed_png(pixel_grid, PNG_PALETTE, png_path)
    print(f'Written {png_path}  ({png_w}x{png_h})')

    write_sprite_files(
        name='sprite',
        frames_top=FRAMES_TOP,
        frames_bottom=FRAMES_BOTTOM,
        palette_colors=PALETTE_COLORS,
        out_dir=out_dir,
    )


if __name__ == '__main__':
    main()
