"""
Player sprite definition for generate_sprites (gen_sprite.py).

SIZE = '16x16': each frame is a flat list of 16 strings, each exactly
16 characters wide.  The builder splits it into 4 tiles:
  left-top  (rows  0-7, cols  0-7)
  left-bot  (rows 8-15, cols  0-7)
  right-top (rows  0-7, cols 8-15)
  right-bot (rows 8-15, cols 8-15)

The character faces RIGHT by default.
Left-facing is achieved in C by setting S_FLIPX on both OBJ slots.

Animation list
--------------
    idle (1 frame)   - stationary stance
    walk (3 frames)  - 3-step walking cycle
    jump (2 frames)  - ascent then fall frames
    die  (1 frame)   - hit by enemy or hazard
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
# Animations are defined as lists of frames, and each frame is a list of 16 strings.
# ---------------------------------------------------------------------------
_FRAME_IDLE = [
    '................', 
    '................',
    '................',
    '................',
    '......DDD.......',
    '.....DDDDDDD....',
    '....DDDYYDY.....',
    '...DDYDDYYYY....',
    '...DDYYYYDDY....',
    '.....YYYYYY.....',
    '....DDBDDB......',
    '...DDDBDDBDD....',
    '...YYDBBBBDDY...',
    '...YYBBBBBBYY...',
    '.....DD..DD.....',
    '....DDD..DDD....',
]
_FRAME_WALK_1 = [
    '................', 
    '................',
    '................',
    '.......DDD......',
    '......DDDDDDD...',
    '.....DDDYYDY....',
    '....DDYDDYYYY...',
    '....DDYYYYDDY...',
    '......YYYYYY....',
    '....DDDDBD......',
    '..YYDDDDBDDYYY..',
    '..YY.DDDBBDDYY..',
    '....BBBBBB..D...',
    '...DBBBBBBBDD...',
    '...DDB...BBDD...',
    '....DDD.........',
]
_FRAME_WALK_2 = [
    '................', 
    '................',
    '................',
    '......DDD.......',
    '.....DDDDDDD....',
    '....DDDYYDY.....',
    '...DDYDDYYYY....',
    '...DDYYYYDDY....',
    '.....YYYYYY.....',
    '....DDD.D.......',
    '...DDDDYDD......',
    '...DDYYBBBB.....',
    '...BBYBBBBB.....',
    '....BBBBDD......',
    '.....DDDDDD.....',
    '.....DDDD.......',
]
_FRAME_WALK_3 = [
    '................', 
    '................',
    '................',
    '................',
    '......DDD.......',
    '.....DDDDDDD....',
    '....DDDYYDY.....',
    '...DDYDDYYYY....',
    '...DDYYYYDDY....',
    '.....YYYYYY.....',
    '....DDDDBD.YY...',
    '...YDDDDDDYYY...',
    '..YYBBBBBBB.....',
    '...DBBBBBB......',
    '..DDBB.DD.......',
    '..D....DDD......',
]
_FRAME_JUMP = [
    '................',
    '................',
    '................',
    '............YYY.',
    '......DDD...YYY.',
    '.....DDDDDDDDD..',
    '....DDDYYDY.DD..',
    '...DDYDDYYYYDD..',
    '...DDYYYYDDDD...',
    '.....YYYYYYD....',
    '...DDDBDDBD.....',
    '.YDDDDBBDDB..D..',
    '.YY.BBBBBBBBDD..',
    '.Y.DBBBBBBBBDD..',
    '..DDDBBBBB......',
    '..D..BB.........',
]
_FRAME_FALL = [
    '................',
    '................',
    '................',
    '......DDD.......',
    '...DDDDDDDD.....',
    '....YDYYDDDD....',
    '...YYYDYDYDYY...',
    '....DDYYYYDYY...',
    '...DDYYYBBB.....',
    '...DDDDDYYBB....',
    '...DDDDDYBBB....',
    '....DDDBDDDB....',
    '......BDDDB.....',
    '......BDDBD.D...',
    '........BDDDD...',
    '.........DDD....',
]
_FRAME_DIE = [
    '................',
    '................',
    '................',
    '......DDDD......',
    '...Y.DDDDDD.Y...',
    '..YYDYDYYDYDYY..',
    '..YDDDYYYYDDDY..',
    '...DDYDDDDYDD...',
    '....DYYYYYYD....',
    '....DDBYYBDD....',
    '..D.BBBDDBBB.D..',
    '..DDDBBBBBBDDD..',
    '..DDDBBBBBBDDD..',
    '.....BBBBBB.....',
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
        _FRAME_IDLE,
    ],
    'walk': [
        _FRAME_WALK_1,
        _FRAME_WALK_2,
        _FRAME_WALK_3,
    ],
    'jump': [
        _FRAME_JUMP,
        _FRAME_FALL,
    ],
    'die': [
        _FRAME_DIE,
    ],
}
