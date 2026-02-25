"""
Player sprite definition for generate_sprites (gen_sprite.py).

Each animation is a list of (top_half, bottom_half) frame pairs.
Both halves are 8 strings of 8 characters; each character maps to a
colour index via PIXEL_CHARS.  '.' is always transparent (index 0).

The character faces RIGHT by default.
Left-facing is achieved in C by setting the S_FLIPX sprite property.

Animation list
--------------
  idle (2 frames)  – stationary breathing bob
  walk (4 frames)  – 4-step walking cycle
  jump (2 frames)  – ascending / descending
  die  (3 frames)  – hit, falling, lying flat
"""

NAME = 'player'

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
# Shared top halves
# ---------------------------------------------------------------------------
_HEAD = [            # head + body (right-facing)
    '..DDDD..',      # head outline top
    '.DYYYYD.',      # face
    '.DYYDYD.',      # face with eye (D at col 4)
    '.DYYYYD.',      # face lower
    '.DBBBD..',      # body top
    'DBBBBD..',      # body with left-arm outline
    '.BBBBD..',      # body
    '.DDDDD..',      # belt
]
_HEAD_JUMP = [       # arms raised for jump
    '..DDDD..',
    '.DYYYYD.',
    '.DYYDYD.',
    '.DYYYYD.',
    'DBBBBD..',      # arms wide
    'DBBBBD..',
    '.BBBBD..',
    '.DDDDD..',
]
_HEAD_HIT = [        # grimace / hit face
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
# Bottom halves
# ---------------------------------------------------------------------------
_LEGS_IDLE = [
    '..BB....',
    '..BB....',
    '..BB....',
    '..BB....',
    '.DDD....',      # boots (both feet together)
    '.DDD....',
    '........',
    '........',
]
_LEGS_WALK1 = [      # right leg forward (left leg stepping back)
    '.B.B....',
    '.B.B....',
    '.B.B....',
    'DB.B....',      # left leg retreats
    'D..B....',
    'D.......',      # only left boot visible at far left
    '........',
    '........',
]
_LEGS_WALK3 = [      # left leg forward (right leg stepping back)
    '.B.B....',
    '.B.B....',
    '.B.B....',
    '.B.BD...',      # right leg retreats
    '...BD...',
    '...D....',      # only right boot visible
    '........',
    '........',
]
_LEGS_JUMP_UP = [    # ascending: knees tucked
    '........',
    '.BBB....',
    '.BBB....',
    '.B.B....',
    'D..D....',      # feet pulled up/back
    'D..D....',
    '........',
    '........',
]
_LEGS_JUMP_DN = [    # descending: legs stretched
    '..BB....',
    '..BB....',
    '..BB....',
    '.B.B....',      # spreading for landing
    '.D.D....',
    '.D.D....',
    '........',
    '........',
]
_LEGS_DIE0 = [       # stagger – one leg buckled
    '..B.....',
    '..B.....',
    '.BB.....',
    '.BB.....',
    '.D......',
    'D.......',
    '........',
    '........',
]
_LEGS_DIE1 = [       # falling sideways
    '.BBB....',
    'BBBB....',
    'BBBB....',
    'BBBB....',
    'DD......',
    'D.......',
    '........',
    '........',
]
_BODY_DIE2_TOP = [   # lying flat – body horizontal
    '........',
    'DDDDDDD.',
    'DYYYYYD.',
    'DBBBBBBD',
    'DDDDDDD.',
    '........',
    '........',
    '........',
]
_BODY_DIE2_BOT = [   # lying flat – legs horizontal
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
        (_HEAD,         _LEGS_IDLE),   # frame 0: normal
        (_HEAD,         _LEGS_IDLE),   # frame 1: (same – subtle; extend for blink if desired)
    ],
    'walk': [
        (_HEAD,         _LEGS_IDLE),   # frame 0: neutral
        (_HEAD,         _LEGS_WALK1),  # frame 1: right leg forward
        (_HEAD,         _LEGS_IDLE),   # frame 2: neutral
        (_HEAD,         _LEGS_WALK3),  # frame 3: left leg forward
    ],
    'jump': [
        (_HEAD_JUMP,    _LEGS_JUMP_UP),  # frame 0: ascending
        (_HEAD_JUMP,    _LEGS_JUMP_DN),  # frame 1: descending
    ],
    'die': [
        (_HEAD_HIT,     _LEGS_DIE0),    # frame 0: hit
        (_HEAD_HIT,     _LEGS_DIE1),    # frame 1: falling
        (_BODY_DIE2_TOP,_BODY_DIE2_BOT),# frame 2: flat on ground
    ],
}
