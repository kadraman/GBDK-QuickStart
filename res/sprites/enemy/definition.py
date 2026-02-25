"""
Enemy sprite definition for generate_sprites (gen_sprite.py).

SIZE = '8x8': each frame is a flat list of 8 strings, each exactly 8
characters wide.  The builder stores [content_tile, blank_tile] so the
visible 8×8 content occupies the TOP half of the 8x16 hardware OBJ slot.
In C, place the enemy OBJ at OAM Y = world_y + 16 where world_y = GROUND_Y
(the ground-level screen Y minus the sprite height).  All 8 pixel rows are
used so the slime body fills the 8×8 grid and sits firmly on the ground.

Animation list
--------------
  idle (2 frames)  – blinking stance
  walk (2 frames)  – simple left/right patrol walk cycle
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

# Animation speeds (vblanks per frame)
ANIM_SPEEDS = {
    'idle': 16,
    'walk': 10,
}

# ---------------------------------------------------------------------------
# Idle frames – slime blinks on frame 1
# All 8 rows used; feet at rows 6-7 for ground contact.
# ---------------------------------------------------------------------------
_IDLE1 = [             # normal open-eye stance
    '..DDDD..',        # 0  head top
    '.DGGGGD.',        # 1  head
    'DGGLGGD.',        # 2  eye highlight
    'DGG.GGD.',        # 3  mouth
    'DGGGGGGD',        # 4  body
    '.DDDDDD.',        # 5  body bottom
    '.GG..GG.',        # 6  feet
    '.D....D.',        # 7  feet – ground contact
]
_IDLE2 = [             # blink – eyes closed
    '..DDDD..',
    '.DGGGGD.',
    'DGDDGGD.',        # 2  eyes closed
    'DGG.GGD.',
    'DGGGGGGD',
    '.DDDDDD.',
    '.GG..GG.',
    '.D....D.',
]

# ---------------------------------------------------------------------------
# Walk frames – squish cycle with feet shifting for movement feel
# ---------------------------------------------------------------------------
_WALK1 = [             # normal stance while moving
    '..DDDD..',
    '.DGGGGD.',
    'DGGLGGD.',
    'DGG.GGD.',
    'DGGGGGGD',
    '.DDDDDD.',
    '.GG..GG.',
    '.D....D.',
]
_WALK2 = [             # slightly squished / alternate eye
    '..DDDD..',
    '.DGGGGD.',
    'DGGDGGD.',        # eye shifted right
    'DGG.GGD.',
    'DGGGGGGD',
    '.DDDDDD.',
    'GG....GG',        # feet spread wider
    'D......D',        # feet wider – ground contact
]

# ---------------------------------------------------------------------------
# Animation table
# ---------------------------------------------------------------------------
ANIMATIONS = {
    'idle': [
        _IDLE1,    # frame 0
        _IDLE2,    # frame 1 – blink
    ],
    'walk': [
        _WALK1,    # frame 0
        _WALK2,    # frame 1
    ],
}
