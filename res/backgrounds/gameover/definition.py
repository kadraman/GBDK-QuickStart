"""
Game Over screen background definition.

Generates: res/bg_gameover.c / res/bg_gameover.h
20x18 tilemap (160x144 px) - fixed screen, no scrolling.

Visual: Ominous crimson sky with dark clouds, cracked dark ground.

Tile index list (8 tiles)
--------------------------
  0  Dark red sky solid
  1  Sky with red cloud streaks
  2  Ominous cloud body
  3  Cloud edge / wisp
  4  Cracked ground top
  5  Cracked ground mid
  6  Dark ground
  7  Deep ground solid

Palette layout
--------------
  Palette 0 (sky / clouds):
    0 = dark crimson  (40, 0, 0)
    1 = medium red    (160, 0, 0)
    2 = bright red    (220, 60, 0)
    3 = near black    (15, 0, 0)

  Palette 1 (ground):
    0 = dark brown    (35, 18, 8)
    1 = crack light   (80, 45, 20)
    2 = mid brown     (55, 28, 12)
    3 = deep dark     (12, 6, 2)
"""

NAME = 'bg_gameover'

# ---------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    # Palette 0: sky / clouds
    ( 40,   0,   0),  # 0 dark crimson
    (160,   0,   0),  # 1 medium red
    (220,  60,   0),  # 2 bright red-orange
    ( 15,   0,   0),  # 3 near black red
    # Palette 1: ground
    ( 35,  18,   8),  # 0 dark brown
    ( 80,  45,  20),  # 1 crack / lighter brown
    ( 55,  28,  12),  # 2 mid brown
    ( 12,   6,   2),  # 3 deep dark
]

PNG_PALETTE = [
    ( 40,   0,   0),  # 0 dark sky
    (160,   0,   0),  # 1 medium red
    (220,  60,   0),  # 2 bright red
    ( 15,   0,   0),  # 3 near black
]

K = 0  # dark sky
R = 1  # red
B = 2  # bright red / orange-red
X = 3  # near black

# ---------------------------------------------------------------------------
# 8x8 tile definitions
# ---------------------------------------------------------------------------
TILES = [
    # 0: Dark red sky solid
    [[K]*8]*8,

    # 1: Sky with red streaks / glow
    [[K,K,R,K,K,K,K,K],
     [K,R,R,R,K,K,K,K],
     [K,K,R,K,K,K,R,K],
     [K,K,K,K,R,R,R,K],
     [K,K,K,K,K,R,K,K],
     [K,K,R,K,K,K,K,K],
     [K,K,K,K,K,K,K,K],
     [K,K,K,R,K,K,K,K]],

    # 2: Ominous cloud body
    [[K,R,R,R,R,R,R,K],
     [R,R,B,B,R,R,R,R],
     [R,B,B,B,B,R,R,R],
     [R,B,B,B,B,B,R,R],
     [R,R,B,B,B,R,R,R],
     [K,R,R,R,R,R,R,K],
     [K,K,K,K,K,K,K,K],
     [K,K,K,K,K,K,K,K]],

    # 3: Cloud edge / wisp
    [[K,K,K,R,R,K,K,K],
     [K,R,R,R,R,R,K,K],
     [R,R,B,R,R,R,R,K],
     [K,R,R,R,R,R,K,K],
     [K,K,K,R,R,K,K,K],
     [K,K,K,K,K,K,K,K],
     [K,K,K,K,K,K,K,K],
     [K,K,K,K,K,K,K,K]],

    # 4: Cracked ground top (uses palette 1)
    # K→0=dark brown, R→1=crack color, B→2=mid brown, X→3=deep dark
    [[K,K,K,K,K,K,K,K],
     [X,X,X,X,X,X,X,X],
     [K,K,K,R,K,K,K,K],
     [K,K,R,K,K,R,K,K],
     [K,R,K,K,K,K,R,K],
     [K,K,K,K,K,K,K,K],
     [B,B,B,B,B,B,B,B],
     [B,B,B,B,B,B,B,B]],

    # 5: Cracked ground mid
    [[B,B,B,B,B,B,B,B],
     [B,B,R,B,B,B,B,B],
     [B,R,K,R,B,B,R,B],
     [B,B,R,B,B,R,K,R],
     [B,B,B,B,R,K,R,B],
     [B,B,B,B,B,R,B,B],
     [B,B,B,B,B,B,B,B],
     [B,B,B,B,B,B,B,B]],

    # 6: Dark ground
    [[X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X],
     [X,X,X,X,X,X,X,X]],

    # 7: Deep ground solid (near black)
    [[X]*8]*8,
]
assert len(TILES) == 8, f"Expected 8 tiles, got {len(TILES)}"

# ---------------------------------------------------------------------------
# 20x18 tilemap
# ---------------------------------------------------------------------------
_SKY   = 0   # dark sky
_STKY  = 1   # sky with streaks
_CLOUD = 2   # cloud body
_WISP  = 3   # cloud wisp
_CRACK = 4   # cracked top
_CRMD  = 5   # cracked mid
_DKGND = 6   # dark ground
_DEEP  = 7   # deep ground

MAP_W, MAP_H = 20, 18

# Cloud clusters: (row, col_start, col_end) using cloud body tiles
_CLOUDS = [
    (2, 3, 6),
    (3, 3, 6),
    (1, 12, 15),
    (2, 12, 15),
    (4, 7, 9),
    (5, 7, 9),
]

def _build_tilemap():
    rows = []
    for row in range(MAP_H):
        r = []
        for col in range(MAP_W):
            tile = _SKY

            if row >= 15:
                tile = _DEEP
            elif row >= 13:
                tile = _DKGND
            elif row == 12:
                tile = _CRMD
            elif row == 11:
                tile = _CRACK
            else:
                # Sky streaks on alternating columns
                if (row * 3 + col * 2) % 7 == 1:
                    tile = _STKY
                # Cloud clusters
                for cr, ccs, cce in _CLOUDS:
                    if row == cr and ccs <= col <= cce:
                        tile = _CLOUD
                        break
                    # Wisp edges around clouds
                    if row == cr and col == ccs - 1:
                        tile = _WISP
                        break
                    if row == cr and col == cce + 1:
                        tile = _WISP
                        break

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
    if row >= 11:
        ATTR_MAP.append(0x01)
    else:
        ATTR_MAP.append(0x00)
