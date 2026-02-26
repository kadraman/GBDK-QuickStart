"""
Win screen background definition.

Generates: res/bg_win.c / res/bg_win.h
20x18 tilemap (160x144 px) - fixed screen, no scrolling.

Visual: Bright golden sky with sun, fluffy white clouds, green hills.

Tile index list (10 tiles)
--------------------------
  0  Golden sky solid
  1  Sky with sparkles
  2  Sun top-left quarter
  3  Sun top-right quarter
  4  Sun bottom-left quarter
  5  Sun bottom-right quarter
  6  Cloud body
  7  Cloud edge (sky around cloud)
  8  Green grass top
  9  Green ground

Palette layout
--------------
  Palette 0 (sky / sun / clouds):
    0 = golden sky  (255,200, 50)
    1 = sun / spark (255,255,200)
    2 = cloud white (240,240,240)
    3 = cloud shadow(180,180,200)

  Palette 1 (ground):
    0 = bright grass (100,220, 80)
    1 = grass mid    ( 70,180, 50)
    2 = dark dirt    ( 80, 60, 20)
    3 = deep dirt    ( 50, 35, 10)
"""

NAME = 'bg_win'

# ---------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------
PALETTE_COLORS = [
    # Palette 0: sky / sun / clouds
    (255, 200,  50),  # 0 golden sky
    (255, 255, 200),  # 1 sun / sparkle bright
    (240, 240, 240),  # 2 cloud white
    (180, 180, 200),  # 3 cloud shadow blue-grey
    # Palette 1: ground
    (100, 220,  80),  # 0 bright green grass
    ( 70, 180,  50),  # 1 grass mid
    ( 80,  60,  20),  # 2 dark dirt
    ( 50,  35,  10),  # 3 deep dirt
]

PNG_PALETTE = [
    (255, 200,  50),  # 0 golden sky
    (255, 255, 200),  # 1 sun / sparkle
    (240, 240, 240),  # 2 cloud white
    (180, 180, 200),  # 3 cloud shadow
]

S = 0  # sky golden
L = 1  # sparkle / sun bright
C = 2  # cloud white
H = 3  # cloud shadow

# ---------------------------------------------------------------------------
# 8x8 tile definitions
# ---------------------------------------------------------------------------
TILES = [
    # 0: Golden sky solid
    [[S]*8]*8,

    # 1: Sky with sparkles (tiny stars/glitter)
    [[S,S,L,S,S,S,S,S],
     [S,S,S,S,S,L,S,S],
     [S,S,S,S,S,S,S,S],
     [S,L,S,S,S,S,S,L],
     [S,S,S,S,L,S,S,S],
     [S,S,S,S,S,S,S,S],
     [L,S,S,S,S,S,L,S],
     [S,S,S,S,S,S,S,S]],

    # 2: Sun top-left quarter
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,L,L,L,L],
     [S,S,S,L,L,L,L,L],
     [S,S,L,L,L,L,L,L],
     [S,S,L,L,L,L,L,L],
     [S,S,L,L,L,L,L,L],
     [S,S,L,L,L,L,L,L]],

    # 3: Sun top-right quarter
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [L,L,L,S,S,S,S,S],
     [L,L,L,L,S,S,S,S],
     [L,L,L,L,L,S,S,S],
     [L,L,L,L,L,S,S,S],
     [L,L,L,L,L,S,S,S],
     [L,L,L,L,L,S,S,S]],

    # 4: Sun bottom-left quarter
    [[S,S,L,L,L,L,L,L],
     [S,S,L,L,L,L,L,L],
     [S,S,L,L,L,L,L,L],
     [S,S,S,L,L,L,L,L],
     [S,S,S,S,L,L,L,L],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 5: Sun bottom-right quarter
    [[L,L,L,L,L,S,S,S],
     [L,L,L,L,L,S,S,S],
     [L,L,L,L,L,S,S,S],
     [L,L,L,L,S,S,S,S],
     [L,L,L,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 6: Cloud body (white puffy cloud)
    [[S,S,C,C,C,C,S,S],
     [S,C,C,C,C,C,C,S],
     [C,C,C,C,C,C,C,C],
     [C,C,C,C,C,C,C,C],
     [C,C,C,C,C,C,C,C],
     [H,H,H,H,H,H,H,H],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 7: Cloud edge / wisp (sky with cloud hint)
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,C,C,S,S],
     [S,S,C,C,C,C,C,S],
     [S,C,C,C,C,C,C,C],
     [H,H,H,H,H,H,H,H],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 8: Green grass top (palette 1)
    [[S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S],
     [S,S,S,S,S,S,S,S]],

    # 9: Green ground solid (palette 1)
    [[H]*8]*8,
]
assert len(TILES) == 10, f"Expected 10 tiles, got {len(TILES)}"

# ---------------------------------------------------------------------------
# 20x18 tilemap (fixed screen, no scrolling)
# ---------------------------------------------------------------------------
_SKY   = 0   # solid golden sky
_SPKL  = 1   # sparkle sky
_SL    = 2   # sun top-left
_SR    = 3   # sun top-right
_SBL   = 4   # sun bottom-left
_SBR   = 5   # sun bottom-right
_CLOUD = 6   # cloud body
_CWSP  = 7   # cloud wisp
_GRASS = 8   # green grass
_GNDDK = 9   # ground

MAP_W, MAP_H = 20, 18

def _build_tilemap():
    rows = []
    for row in range(MAP_H):
        r = []
        for col in range(MAP_W):
            tile = _SKY

            if row < 14:
                # Sparkle pattern in sky
                if (row + col) % 4 == 1:
                    tile = _SPKL
                # Sun at rows 1-2, cols 1-2
                if   row == 1 and col == 1: tile = _SL
                elif row == 1 and col == 2: tile = _SR
                elif row == 2 and col == 1: tile = _SBL
                elif row == 2 and col == 2: tile = _SBR
                # Cloud 1 at rows 4-5, cols 6-9
                elif row in (4, 5) and 6 <= col <= 9:
                    tile = _CLOUD
                # Cloud 2 at rows 7-8, cols 13-17
                elif row in (7, 8) and 13 <= col <= 17:
                    tile = _CLOUD
                elif row == 6 and col in (7, 8):
                    tile = _CWSP
                elif row == 9 and col in (14, 15):
                    tile = _CWSP
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
