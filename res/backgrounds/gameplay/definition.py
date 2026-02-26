"""
Gameplay background definition.

Generates: res/background.c / res/background.h
48x18 tilemap (384x144 px). The visible GBC screen is 160 px (20 tiles);
the extra 28 tiles provide 224 pixels of smooth horizontal scroll.

Column streaming is required in gameplay C code to handle the 48-tile map
in the 32-tile-wide GBC hardware background ring buffer.

Tile index list (16 tiles)
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
 11  Tree trunk
 12  Grass top
 13  Dirt
 14  Deep ground
 15  Platform block (stone, player can land on top)

Palette layout
--------------
  Palette 0 (sky/clouds/foliage, rows 0-9):
    0 = sky blue  (155,200,234)
    1 = white     (255,255,255)   clouds
    2 = tree green(60,150,60)     foliage
    3 = tree dark (40,80,20)      foliage shadow / trunk

  Palette 1 (ground/grass/platform, rows 10-17 + platform tiles):
    0 = grass light(120,200,80)
    1 = grass mid  (80,160,80)
    2 = dirt/stone brown (120,80,40)
    3 = dirt dark  (80,50,20)
"""

NAME = 'bg_gameplay'

# ---------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    # Palette 0: sky / clouds / tree foliage
    (155, 200, 234),  # 0 sky blue
    (255, 255, 255),  # 1 cloud white
    ( 60, 150,  60),  # 2 tree green
    ( 40,  80,  20),  # 3 tree dark / trunk
    # Palette 1: ground / grass / dirt / platform
    (120, 200,  80),  # 0 grass light
    ( 80, 160,  80),  # 1 grass mid
    (120,  80,  40),  # 2 dirt/stone brown
    ( 80,  50,  20),  # 3 dirt dark
]

PNG_PALETTE = [
    (155, 200, 234),  # 0 sky blue
    (255, 255, 255),  # 1 white / cloud
    ( 60, 150,  60),  # 2 green
    ( 40,  80,  20),  # 3 dark
]

S = 0  # sky
W = 1  # white / cloud
G = 2  # green (foliage / grass)
D = 3  # dark  (tree outline / dirt)

# ---------------------------------------------------------------------------
# 8x8 tile definitions (16 tiles)
# ---------------------------------------------------------------------------
TILES = [
    # 0: Sky solid
    [[S]*8]*8,

    # 1: Cloud top-left
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W],
     [S,S,W,W,W,W,W,W]],

    # 2: Cloud top-centre
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

    # 7: Tree foliage top-left
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

    # 11: Tree trunk
    [[S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S],
     [S,S,S,D,D,S,S,S]],

    # 12: Grass top
    [[G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G],
     [G,G,G,G,G,G,G,G]],

    # 13: Dirt
    [[D,D,D,D,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [D,D,S,D,D,D,S,D],
     [D,D,D,D,D,D,D,D],
     [D,D,D,S,D,D,D,D],
     [D,D,D,D,D,D,D,D],
     [S,D,D,D,D,D,S,D],
     [D,D,D,D,D,D,D,D]],

    # 14: Deep ground
    [[D]*8]*8,

    # 15: Platform block (stone) – uses palette 1 (attr=0x01)
    #     Colors map to palette-1: 0=grass-light, 1=grass-mid, 2=dirt-brown, 3=dirt-dark
    #     We use index 2 (brown) and 3 (dark) for stone look
    [[D,D,D,D,D,D,D,D],   # D→3 = dirt dark (outline)
     [D,G,G,D,G,G,G,D],   # G→2 = dirt brown (stone face)
     [D,G,G,D,G,G,G,D],
     [D,D,D,D,D,D,D,D],
     [D,G,G,G,D,G,G,D],
     [D,G,G,G,D,G,G,D],
     [D,G,G,G,D,G,G,D],
     [D,D,D,D,D,D,D,D]],
]
assert len(TILES) == 16, f"Expected 16 tiles, got {len(TILES)}"

# ---------------------------------------------------------------------------
# 48x18 tilemap
# ---------------------------------------------------------------------------
_SKY    = 0
_CTL,_CTC,_CTR = 1,2,3   # cloud top
_CBL,_CBC,_CBR = 4,5,6   # cloud bottom
_FTL,_FTR = 7,8           # foliage top
_FBL,_FBR = 9,10          # foliage bottom
_TRK    = 11               # trunk
_GRASS  = 12
_DIRT   = 13
_DEEP   = 14
_PLAT   = 15               # platform block

MAP_W, MAP_H = 48, 18

# Trees: (left_col, right_col)
_TREES = [(4,5), (17,18), (31,32), (44,45)]

# Pits: columns where ground is missing (world col ranges, inclusive)
# PIT1: cols 10-12 (3 tiles)
# PIT2: cols 21-24 (4 tiles)
# PIT3: cols 34-38 (5 tiles, wide jump)
_PITS = [(10,12), (21,24), (34,38)]

# Platform blocks: cols where row 9 has a platform tile
# Placed on solid ground sections, before or between pits
_PLAT_COLS = {7, 15, 16, 27, 28, 42}

def _is_pit(col):
    for (ps, pe) in _PITS:
        if ps <= col <= pe:
            return True
    return False

def _build_tilemap():
    rows = []
    for row in range(MAP_H):
        r = []
        for col in range(MAP_W):
            tile = _SKY

            pit = _is_pit(col)

            if not pit:
                if row >= 14:
                    tile = _DEEP
                elif row >= 11:
                    tile = _DIRT
                elif row == 10:
                    tile = _GRASS
                elif row == 9 and col in _PLAT_COLS:
                    tile = _PLAT
                else:
                    # Clouds
                    if row == 2 and 3 <= col <= 5:
                        tile = [_CTL,_CTC,_CTR][col-3]
                    elif row == 3 and 3 <= col <= 5:
                        tile = [_CBL,_CBC,_CBR][col-3]
                    elif row == 1 and 25 <= col <= 27:
                        tile = [_CTL,_CTC,_CTR][col-25]
                    elif row == 2 and 25 <= col <= 27:
                        tile = [_CBL,_CBC,_CBR][col-25]
                    elif row == 1 and 38 <= col <= 40:
                        tile = [_CTL,_CTC,_CTR][col-38]
                    elif row == 2 and 38 <= col <= 40:
                        tile = [_CBL,_CBC,_CBR][col-38]
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
ATTR_MAP = []
for i in range(MAP_W * MAP_H):
    row = i // MAP_W
    col = i % MAP_W
    # Platform tiles use palette 1; ground rows use palette 1; sky rows palette 0
    if TILEMAP_FLAT[i] == _PLAT:
        ATTR_MAP.append(0x01)
    elif row < 10:
        ATTR_MAP.append(0x00)
    else:
        ATTR_MAP.append(0x01)
