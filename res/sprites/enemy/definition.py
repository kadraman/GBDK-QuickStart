"""
Enemy sprite definition for generate_sprites (gen_sprite.py).

SIZE = '8x8': each frame is a 1-tuple (content_rows,).
  content_rows : 8 strings of 8 characters.

Stored as [content_tile, blank_tile] pairs so that the visible 8x8
content occupies the TOP half of the 8x16 hardware sprite slot.
In C, place the enemy OBJ at OAM Y = GROUND_Y_HW + 8 so its feet
align with the ground surface (screen_y bottom = GROUND_Y_HW + 8 - 16 + 8
= GROUND_Y_HW).

Animation list
--------------
  walk (2 frames) – simple left/right patrol walk cycle
"""

NAME = 'enemy'
SIZE = '8x8'

# GBC sprite palette: index 0 = transparent on OBJ layer
PALETTE = [
    (255,   0, 255),   # 0 – transparent (magenta key)
    ( 20,  20,  20),   # 1 – dark outline (D)
    (100, 180,  50),   # 2 – green body (G)
    (160, 230,  80),   # 3 – light green highlight (L)
]

# Character → colour index.  '.' is always 0 (transparent).
PIXEL_CHARS = {'.': 0, 'D': 1, 'G': 2, 'L': 3}

# ---------------------------------------------------------------------------
# Walk frames (2-frame squish cycle)
# ---------------------------------------------------------------------------
_WALK1 = [             # normal stance
    '..DDDD..',
    '.DGGGGD.',
    'DGGLGGD.',
    'DGG.GGD.',
    'DGGGGGGD',
    '.DDDDDD.',
    '........',
    '........',
]
_WALK2 = [             # slightly squished / alternate eye
    '..DDDD..',
    '.DGGGGD.',
    'DGGDGGD.',
    'DGG.GGD.',
    'DGGGGGGD',
    '.DDDDDD.',
    '........',
    '........',
]

# ---------------------------------------------------------------------------
# Animation table
# ---------------------------------------------------------------------------
ANIMATIONS = {
    'walk': [
        (_WALK1,),   # frame 0
        (_WALK2,),   # frame 1
    ],
}
