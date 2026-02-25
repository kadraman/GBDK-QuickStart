"""
Player sprite definition for generate_sprites (gen_sprite.py).

SIZE = '16x16': each frame is a 4-tuple (l_top, l_bot, r_top, r_bot).
  l_top / l_bot  : left  8x8 half (top and bottom rows)
  r_top / r_bot  : right 8x8 half (transparent – occupies 16x16 OBJ space)

Each half is 8 strings of 8 characters; each character maps to a
colour index via PIXEL_CHARS.  '.' is always transparent (index 0).

The character faces RIGHT by default.
Left-facing is achieved in C by setting the S_FLIPX sprite property
on both OBJ slots.

Pixels are placed in the lower half of the 16x16 grid (head in l_top,
legs/feet in l_bot row 7) so the sprite appears grounded at GROUND_Y.

Animation list
--------------
  idle (2 frames)  – stationary breathing bob
  walk (4 frames)  – 4-step walking cycle
  jump (2 frames)  – ascending / descending
  die  (3 frames)  – hit, falling, lying flat
"""

NAME = 'player'
SIZE = '16x16'

# GBC sprite palette: index 0 = transparent on OBJ layer
PALETTE = [
    (255,   0, 255),   # 0 – transparent (magenta key)
    (240, 180,  80),   # 1 – skin  (Y)
    ( 50, 100, 200),   # 2 – blue outfit (B)
    ( 20,  20,  20),   # 3 – dark outline (D)
]

# Character → colour index.  '.' is always 0 (transparent).
PIXEL_CHARS = {'.': 0, 'Y': 1, 'B': 2, 'D': 3}

# ---------------------------------------------------------------------------
# Right half is transparent – provides 16-wide OBJ space while keeping art
# ---------------------------------------------------------------------------
_EMPTY = [
    '........',
    '........',
    '........',
    '........',
    '........',
    '........',
    '........',
    '........',
]

# ---------------------------------------------------------------------------
# Left half top (head + upper body, 8x8)
# ---------------------------------------------------------------------------
_L_HEAD = [            # head + body (right-facing)
    '..DDDD..',      # head outline top
    '.DYYYYD.',      # face
    '.DYYDYD.',      # face with eye (D at col 4)
    '.DYYYYD.',      # face lower
    '.DBBBD..',      # body top
    'DBBBBD..',      # body with left-arm outline
    '.BBBBD..',      # body
    '.DDDDD..',      # belt
]
_L_HEAD_JUMP = [       # arms raised for jump
    '..DDDD..',
    '.DYYYYD.',
    '.DYYDYD.',
    '.DYYYYD.',
    'DBBBBD..',      # arms wide
    'DBBBBD..',
    '.BBBBD..',
    '.DDDDD..',
]
_L_HEAD_HIT = [        # grimace / hit face
    '..DDDD..',
    '.DDDDD..',      # furrowed brow
    '.DYYDYD.',
    '.DDDDD..',      # clenched mouth
    '.DBBBD..',
    'DBBBBD..',
    '.BBBBD..',
    '.DDDDD..',
]

# ---------------------------------------------------------------------------
# Left half bottom (legs, 8x8)
# ---------------------------------------------------------------------------
_L_LEGS_IDLE = [
    '..BB....',
    '..BB....',
    '..BB....',
    '..BB....',
    '.DDD....',      # boots (both feet together)
    '.DDD....',
    '........',
    '........',
]
_L_LEGS_WALK1 = [      # right leg forward (left leg stepping back)
    '.B.B....',
    '.B.B....',
    '.B.B....',
    'DB.B....',      # left leg retreats
    'D..B....',
    'D.......',      # only left boot visible at far left
    '........',
    '........',
]
_L_LEGS_WALK3 = [      # left leg forward (right leg stepping back)
    '.B.B....',
    '.B.B....',
    '.B.B....',
    '.B.BD...',      # right leg retreats
    '...BD...',
    '...D....',      # only right boot visible
    '........',
    '........',
]
_L_LEGS_JUMP_UP = [    # ascending: knees tucked
    '........',
    '.BBB....',
    '.BBB....',
    '.B.B....',
    'D..D....',      # feet pulled up/back
    'D..D....',
    '........',
    '........',
]
_L_LEGS_JUMP_DN = [    # descending: legs stretched
    '..BB....',
    '..BB....',
    '..BB....',
    '.B.B....',      # spreading for landing
    '.D.D....',
    '.D.D....',
    '........',
    '........',
]
_L_LEGS_DIE0 = [       # stagger – one leg buckled
    '..B.....',
    '..B.....',
    '.BB.....',
    '.BB.....',
    '.D......',
    'D.......',
    '........',
    '........',
]
_L_LEGS_DIE1 = [       # falling sideways
    '.BBB....',
    'BBBB....',
    'BBBB....',
    'BBBB....',
    'DD......',
    'D.......',
    '........',
    '........',
]
_L_BODY_DIE2_TOP = [   # lying flat – body horizontal
    '........',
    'DDDDDDD.',
    'DYYYYYD.',
    'DBBBBBBD',
    'DDDDDDD.',
    '........',
    '........',
    '........',
]
_L_BODY_DIE2_BOT = [   # lying flat – legs horizontal
    'DDDDDDD.',
    '........',
    '........',
    '........',
    '........',
    '........',
    '........',
    '........',
]

# ---------------------------------------------------------------------------
# Animation table
# ---------------------------------------------------------------------------
ANIMATIONS = {
    'idle': [
        (_L_HEAD,          _L_LEGS_IDLE,   _EMPTY, _EMPTY),  # frame 0
        (_L_HEAD,          _L_LEGS_IDLE,   _EMPTY, _EMPTY),  # frame 1
    ],
    'walk': [
        (_L_HEAD,          _L_LEGS_IDLE,   _EMPTY, _EMPTY),  # frame 0: neutral
        (_L_HEAD,          _L_LEGS_WALK1,  _EMPTY, _EMPTY),  # frame 1: right leg forward
        (_L_HEAD,          _L_LEGS_IDLE,   _EMPTY, _EMPTY),  # frame 2: neutral
        (_L_HEAD,          _L_LEGS_WALK3,  _EMPTY, _EMPTY),  # frame 3: left leg forward
    ],
    'jump': [
        (_L_HEAD_JUMP,     _L_LEGS_JUMP_UP, _EMPTY, _EMPTY), # frame 0: ascending
        (_L_HEAD_JUMP,     _L_LEGS_JUMP_DN, _EMPTY, _EMPTY), # frame 1: descending
    ],
    'die': [
        (_L_HEAD_HIT,      _L_LEGS_DIE0,    _EMPTY, _EMPTY), # frame 0: hit
        (_L_HEAD_HIT,      _L_LEGS_DIE1,    _EMPTY, _EMPTY), # frame 1: falling
        (_L_BODY_DIE2_TOP, _L_BODY_DIE2_BOT,_EMPTY, _EMPTY), # frame 2: flat
    ],
}
