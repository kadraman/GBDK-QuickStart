"""
Title screen background definition.

Generates: res/bg_title.c / res/bg_title.h
20x18 tilemap (160x144 px) - fixed screen, no scrolling.

Visual: Starry night sky with moon, mountain silhouettes, dark grass.

Tile index list (10 tiles)
--------------------------
  0  Night sky solid (deep navy)
  1  Sky with stars
  2  Sky with moon (top-left quarter)
  3  Sky with moon (top-right quarter)
  4  Sky with moon (bottom-left quarter)
  5  Sky with moon (bottom-right quarter)
  6  Mountain peak (sky background)
  7  Mountain body (solid)
  8  Dark grass top
  9  Dark ground (dirt/soil)

Palette layout
--------------
  Palette 0 (sky / moon / stars / mountains):
    0 = night sky (0,0,60)    deep navy
    1 = moon/star (255,255,180)  warm white
    2 = mountain  (30,30,100)   blue-grey silhouette
    3 = mtn dark  (15,15,50)    near-black navy

  Palette 1 (ground):
    0 = dark grass (40,80,20)
    1 = grass mid  (30,60,15)
    2 = dark dirt  (50,30,10)
    3 = deep dark  (20,10,5)
"""

NAME = 'bg_title'

# ---------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    # Palette 0: sky / moon / stars / mountains
    (  0,   0,  60),  # 0 night sky deep navy
    (255, 255, 180),  # 1 moon / star warm white
    ( 30,  30, 100),  # 2 mountain blue-grey
    ( 15,  15,  50),  # 3 mountain very dark
    # Palette 1: ground
    ( 40,  80,  20),  # 0 dark grass
    ( 30,  60,  15),  # 1 grass mid dark
    ( 50,  30,  10),  # 2 dark dirt
    ( 20,  10,   5),  # 3 deep dark ground
]

PNG_PALETTE = [
    (  0,   0,  60),  # 0 night sky
    (255, 255, 180),  # 1 moon / star white
    ( 30,  30, 100),  # 2 mountain
    ( 15,  15,  50),  # 3 dark
]

N = 0  # night sky
M = 1  # moon / star bright
U = 2  # mountain
D = 3  # dark / near black

# ---------------------------------------------------------------------------
# 8x8 tile definitions
# ---------------------------------------------------------------------------
TILES = [
    # 0: Night sky solid
    [[N]*8]*8,

    # 1: Sky with scattered stars
    [[N,N,M,N,N,N,N,N],
     [N,N,N,N,N,N,M,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,M,N,N,N],
     [M,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,M],
     [N,N,N,M,N,N,N,N],
     [N,N,N,N,N,N,N,N]],

    # 2: Moon top-left quarter (crescent/circle top-left)
    [[N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,M,M,M,M],
     [N,N,N,M,M,M,M,M],
     [N,N,M,M,M,M,M,M],
     [N,N,M,M,M,M,M,M],
     [N,N,M,M,M,M,M,M],
     [N,N,M,M,M,M,M,M]],

    # 3: Moon top-right quarter
    [[N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [M,M,M,N,N,N,N,N],
     [M,M,M,M,N,N,N,N],
     [M,M,M,M,M,N,N,N],
     [M,M,M,M,M,N,N,N],
     [M,M,M,M,M,N,N,N],
     [M,M,M,M,M,N,N,N]],

    # 4: Moon bottom-left quarter
    [[N,N,M,M,M,M,M,M],
     [N,N,M,M,M,M,M,M],
     [N,N,M,M,M,M,M,M],
     [N,N,N,M,M,M,M,M],
     [N,N,N,N,M,M,M,M],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N]],

    # 5: Moon bottom-right quarter
    [[M,M,M,M,M,N,N,N],
     [M,M,M,M,M,N,N,N],
     [M,M,M,M,M,N,N,N],
     [M,M,M,M,N,N,N,N],
     [M,M,M,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N]],

    # 6: Mountain peak (sky surrounds triangle peak)
    [[N,N,N,N,U,N,N,N],
     [N,N,N,U,U,U,N,N],
     [N,N,U,U,U,U,U,N],
     [N,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U]],

    # 7: Mountain body (filled silhouette)
    [[U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U],
     [U,U,U,U,U,U,U,U]],

    # 8: Dark grass top (uses palette 1)
    [[N,N,N,N,N,N,N,N],  # N→0=dark grass in pal1
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N],
     [N,N,N,N,N,N,N,N]],

    # 9: Dark ground (uses palette 1, color D→3=deep dark)
    [[D]*8]*8,
]
assert len(TILES) == 10, f"Expected 10 tiles, got {len(TILES)}"

# ---------------------------------------------------------------------------
# 20x18 tilemap (fixed screen, no scrolling)
# ---------------------------------------------------------------------------
_NSKY  = 0   # night sky
_STAR  = 1   # star sky
_ML    = 2   # moon top-left
_MR    = 3   # moon top-right
_BL    = 4   # moon bottom-left
_BR    = 5   # moon bottom-right
_MPEAK = 6   # mountain peak
_MBODY = 7   # mountain body
_GRASS = 8   # dark grass
_GNDDK = 9   # dark ground

MAP_W, MAP_H = 20, 18

def _build_tilemap():
    rows = []
    for row in range(MAP_H):
        r = []
        for col in range(MAP_W):
            tile = _NSKY

            # Sky / stars (rows 0-13)
            if row < 14:
                # Star pattern (checkerboard-ish)
                if (row + col) % 5 == 2:
                    tile = _STAR
                # Moon at rows 2-3, cols 14-15 (2x2 tiles)
                if row == 2 and col == 14: tile = _ML
                elif row == 2 and col == 15: tile = _MR
                elif row == 3 and col == 14: tile = _BL
                elif row == 3 and col == 15: tile = _BR
                # Mountain range 1 (rows 9-13, cols 0-5)
                elif row == 9 and col in (2, 7, 14):
                    tile = _MPEAK
                elif row >= 10 and col <= 5:
                    tile = _MBODY
                elif row >= 10 and col == 6:
                    tile = _MBODY
                elif row >= 11 and 7 <= col <= 12:
                    tile = _MBODY
                elif row == 10 and col in (7, 8):
                    tile = _MBODY
                elif row == 10 and col == 7:
                    tile = _MPEAK
                elif row >= 12 and 13 <= col <= 19:
                    tile = _MBODY
                elif row == 11 and 13 <= col <= 19:
                    tile = _MBODY
                elif row == 10 and 14 <= col <= 19:
                    tile = _MBODY
            elif row == 14:
                tile = _GRASS
            else:
                tile = _GNDDK

            r.append(tile)
        rows.append(r)
    return [t for row in rows for t in row]

TILEMAP_FLAT = _build_tilemap()
assert len(TILEMAP_FLAT) == MAP_W * MAP_H

# ---------------------------------------------------------------------------
# Per-tile palette attribute map
# ---------------------------------------------------------------------------
ATTR_MAP = []
for i in range(MAP_W * MAP_H):
    row = i // MAP_W
    # Ground rows use palette 1
    if row >= 14:
        ATTR_MAP.append(0x01)
    else:
        ATTR_MAP.append(0x00)
