"""
Player sprite definition for generate_sprites (gen_sprite.py).

SIZE = '16x16': each frame is a flat list of 16 strings, each exactly
16 characters wide.  The builder splits it into 4 tiles:
  left-top  (rows  0-7, cols  0-7)
  left-bot  (rows 8-15, cols  0-7)
  right-top (rows  0-7, cols 8-15)
  right-bot (rows 8-15, cols 8-15)

Art occupies the left half; the right half is transparent, giving the
sprite its 16-wide OBJ canvas while keeping art in 8 columns.

Pixels are placed in the lower portion of the 16×16 grid (row 15 =
ground contact) so the player appears to stand on the ground.

The character faces RIGHT by default.
Left-facing is achieved in C by setting S_FLIPX on both OBJ slots.

Animation list
--------------
  idle (2 frames)  – stationary stance
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
# Shared top section (rows 0-8): head + upper body
# Each list is 9 strings of 16 chars.
# ---------------------------------------------------------------------------
_TOP_NORMAL = [
    '................',   # row  0  sky
    '................',   # row  1  sky
    '...DDDD.........',   # row  2  head outline top
    '..DYYYYD........',   # row  3  face
    '..DYYDYD........',   # row  4  face with eye
    '..DYYYYD........',   # row  5  face lower
    '..DBBBD.........',   # row  6  body top
    '.DBBBBD.........',   # row  7  body + left-arm outline
    '..BBBBD.........',   # row  8  body
]
_TOP_JUMP = [
    '................',
    '................',
    '...DDDD.........',
    '..DYYYYD........',
    '..DYYDYD........',
    '..DYYYYD........',
    '.DBBBBD.........',   # arms wide
    '.DBBBBD.........',
    '..BBBBD.........',
]
_TOP_HIT = [
    '................',
    '................',
    '...DDDD.........',
    '..DDDDD.........',   # furrowed brow
    '..DYYDYD........',
    '..DDDDD.........',   # clenched mouth
    '..DBBBD.........',
    '.DBBBBD.........',
    '..BBBBD.........',
]

# ---------------------------------------------------------------------------
# Shared bottom section (rows 9-15): belt + legs + boots
# Each list is 7 strings of 16 chars.
# ---------------------------------------------------------------------------
_BOT_IDLE = [
    '..DDDDD.........',   # row  9  belt
    '...BB...........',   # row 10  legs
    '...BB...........',   # row 11
    '...BB...........',   # row 12
    '...BB...........',   # row 13
    '..DDDD..........',   # row 14  boots
    '..DDDD..........',   # row 15  boots – ground contact
]
_BOT_WALK1 = [           # right leg forward, left leg back
    '..DDDDD.........',
    '..B.B...........',
    '..B.B...........',
    '..B.B...........',
    '.DB.B...........',
    '.D..B...........',
    '.D..............',
]
_BOT_WALK3 = [           # left leg forward, right leg back
    '..DDDDD.........',
    '..B.B...........',
    '..B.B...........',
    '..B.B...........',
    '..B.BD..........',
    '...BD...........',
    '...D............',
]
_BOT_JUMP_UP = [         # ascending: knees tucked
    '..DDDDD.........',
    '................',
    '..BBB...........',
    '..BBB...........',
    '..B.B...........',
    '.D..D...........',
    '.D..D...........',
]
_BOT_JUMP_DN = [         # descending: legs stretched
    '..DDDDD.........',
    '...BB...........',
    '...BB...........',
    '...BB...........',
    '..B.B...........',
    '..D.D...........',
    '..D.D...........',
]
_BOT_DIE0 = [            # stagger – one leg buckled
    '..DDDDD.........',
    '...B............',
    '...B............',
    '..BB............',
    '..BB............',
    '..D.............',
    '.D..............',
]
_BOT_DIE1 = [            # falling sideways
    '..DDDDD.........',
    '..BBB...........',
    '.BBBB...........',
    '.BBBB...........',
    '.BBBB...........',
    '.DD.............',
    '.D..............',
]

# Die-2 is a special flat-on-ground pose using a single 16-row frame
_DIE2 = [
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '.DDDDDDD........',   # body outline (horizontal)
    'DYYYYYDDD.......',   # head + body + legs
    '.DBBBBBBD.......',
    '.DDDDDDD........',
    '................',
    '................',
    '................',
    '................',
    '................',
]

# ---------------------------------------------------------------------------
# Animation speeds (vblanks per frame)
# ---------------------------------------------------------------------------
ANIM_SPEEDS = {
    'idle': 20,
    'walk':  8,
    'jump':  1,
    'die':  20,
}

# ---------------------------------------------------------------------------
# Animation table – each frame is a 16-row list
# ---------------------------------------------------------------------------
ANIMATIONS = {
    'idle': [
        _TOP_NORMAL + _BOT_IDLE,   # frame 0
        _TOP_NORMAL + _BOT_IDLE,   # frame 1 (same – extend for blink if desired)
    ],
    'walk': [
        _TOP_NORMAL + _BOT_IDLE,   # frame 0: neutral
        _TOP_NORMAL + _BOT_WALK1,  # frame 1: right leg forward
        _TOP_NORMAL + _BOT_IDLE,   # frame 2: neutral
        _TOP_NORMAL + _BOT_WALK3,  # frame 3: left leg forward
    ],
    'jump': [
        _TOP_JUMP + _BOT_JUMP_UP,  # frame 0: ascending
        _TOP_JUMP + _BOT_JUMP_DN,  # frame 1: descending
    ],
    'die': [
        _TOP_HIT  + _BOT_DIE0,     # frame 0: hit
        _TOP_HIT  + _BOT_DIE1,     # frame 1: falling
        _DIE2,                      # frame 2: flat on ground
    ],
}
