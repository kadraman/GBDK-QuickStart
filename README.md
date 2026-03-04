# GBDK QuickStart

This repository contains a QuickStart template for writing **GameBoy Color (GBC)** games, using [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020). 
It Includes a state-machine game loop, pre-generated 2bpp tile assets, GBC color palettes, animated sprites, and VS Code tooling.

---

## Features

- **Reusable C library (src/lib)**: `sprite` (sprite struct + collision helpers), `sprite_manager` (fixed-size pool, alloc/free/update_hw), `state_machine` (simple GameState framework), and `utils` (drawing helpers). Public headers live in `src/lib/include`.
- **Game application (src/game)**: `main.c`, state implementations (title, gameplay, gameover, win), and game-specific sprite modules (`sprite_player`, `sprite_enemy`) that consume the reusable library.
- **Sprite & animation**: 8×16 sprite support, per-sprite tile base, frames-per-animation, flip and palette control, and hardware OAM placement helpers.
- **Collision & pooling**: AABB collision helper `sprites_collide()` and a small sprite pool (`SPRITE_MANAGER_MAX`) for predictable memory/OBJ usage.
- **GBC color support**: background and sprite palette setup, VRAM bank attribute writes (VBK_REG), and example HUD window palettes.
- **Multiple named backgrounds**: One `res/backgrounds/<name>/definition.py` per state produces `res/<name>.c/.h`. States load their own tiles and palettes on `init()` to provide distinct themed visuals (night sky for title, crimson for game-over, golden for win, scrolling 48-tile level for gameplay).
- **Multiple fonts**: Font definitions in `res/fonts/<name>/definition.py`, same auto-discovery as backgrounds and sprites.
- **Timer HUD**: A 60-second countdown (`TIME: XX`) displayed in the HUD during gameplay; reaching zero triggers game-over.  The HUD is drawn in a window; sprite code hides the player when it falls beneath the HUD to avoid rendering artifacts (window layers are always on top).
- **Wide pitfall level**: 48-tile (384 px) scrolling level with 3 pit zones, 4 raised platforms, column streaming into the 32-tile hardware ring buffer, and a **finish flag** at the far right that triggers the win state.
- **Asset tooling**: Python generators in `tools/` to produce indexed PNGs and `.c/.h` asset files; optional `png2asset` conversion via Makefile.  Each background `definition.py` exports two tile-ID lists: `COLLISION_TILE_IDS` (multi-directional — block all sides, used for walls and solid ground) and `COLLISION_TILE_DOWN_IDS` (landing-surface only — sprites can pass through from below or the sides, used for one-way air platforms).  The per-tile `ATTR_MAP` controls which GBC background palette is applied to each tile position.
- **Modular includes**: Makefile adds `-Isrc/lib/include` and `-Ires` so code can `#include "sprite.h"` and `#include "bg_gameplay.h"` without path noise.

## Prerequisites

| Tool | Purpose | Download |
|---|---|---|
| **GBDK-2020** | Compiler & linker (`lcc`) | https://github.com/gbdk-2020/gbdk-2020/releases |
| **Emulicious** | Emulator for testing | https://emulicious.net |
| **Python 3 + Pillow** | `make generate` (regenerate PNG/C/H assets). The Makefile auto-detects `python3` or `python`; override with `make PYTHON=python3` if needed. | `pip install pillow` |

Install GBDK-2020 to `~/gbdk/` (default), or set the `GBDK_HOME` environment variable:

```bash
export GBDK_HOME=/path/to/gbdk
```

---

## Project Structure

```
GBDK-QuickStart/
├── src/
│   ├── lib/                  # Reusable library code (public headers + impl)
│   │   ├── include/          # Public API headers (add -Isrc/lib/include)
│   │   │   ├── sprite.h
│   │   │   ├── sprite_manager.h
│   │   │   ├── states.h
│   │   │   └── utils.h
│   │   └── src/              # Library implementations
│   │       ├── sprite.c
│   │       ├── sprite_manager.c
│   │       ├── state_machine.c
│   │       └── utils.c
│   └── game/                 # Application / game-specific code
│       ├── main.c            # Entry: VRAM setup, palettes, main loop
│       ├── states/           # State implementations (game logic)
│       │   ├── state_title.c
│       │   ├── state_gameplay.c
│       │   ├── state_gameover.c
│       │   └── state_win.c
│       └── sprites/          # Game-specific sprite modules
│           ├── sprite_player.c
│           └── sprite_enemy.c
├── res/                     # Generated assets (PNG + .c/.h from generators)
│   ├── backgrounds/          # Background definitions (one sub-dir per state)
│   │   ├── gameplay/definition.py  → bg_gameplay.c/.h (48-tile wide level)
│   │   ├── title/definition.py     → bg_title.c/.h    (night sky)
│   │   ├── gameover/definition.py  → bg_gameover.c/.h (crimson sky)
│   │   └── win/definition.py       → bg_win.c/.h      (golden sky)
│   ├── fonts/
│   │   └── default/definition.py  → font.c/.h
│   ├── sprites/
│   │   ├── player/definition.py   → player.c/.h (16x16 animated)
│   │   └── enemy/definition.py    → enemy.c/.h  (8x8 patrol enemy)
│   ├── bg_gameplay.png / bg_gameplay.c/.h
│   ├── bg_title.png / bg_title.c/.h
│   ├── bg_gameover.png / bg_gameover.c/.h
│   ├── bg_win.png / bg_win.c/.h
│   ├── font.png / font.c/.h
│   ├── player.png / player.c/.h
│   └── enemy.png / enemy.c/.h
├── tools/                   # Asset generation scripts (Python)
├── .vscode/
├── Makefile                 # Build system (now sets include flags for lib/game/res)
└── README.md
```

Notes:
- Public library headers live in `src/lib/include`. The `Makefile` already
   sets compiler `-I` include paths for `src/lib/include`, `src/game`,
   `src/game/states`, `src/game/sprites` and `res/` so sources may simply
   `#include "sprite.h"` or `#include "bg_gameplay.h"` as seen in the code.
- Reusable logic (sprite manager, collision, state machine, utils) is in
   `src/lib/*` and can be extracted to separate projects or published as a
   small library in future.


---

## Build Instructions

### 1. Build the ROM

```bash
make all
# Output: obj/quickstart.gbc
```

### 2. Run in Emulicious

```bash
make run
# or Emulicious obj/quickstart.gbc
```

On Windows the Makefile defaults `EMULICIOUS` to `Emulicious.exe`; on Unix-like systems it will prefer a system `Emulicious` binary or fall back to `java -jar Emulicious.jar`. Override with `make EMULICIOUS="java -jar /path/to/Emulicious.jar" run` if required.

### 3. Regenerate assets from Python (no GBDK needed)

If you edit the PNG files or the Python generator scripts, regenerate all C/H
source files and PNGs with:

```bash
make generate
# or: python tools/generate_assets.py
```

Notes:
- `make generate` auto-detects the Python command; to force a specific interpreter use `make PYTHON=python3 generate`.
- The generator scripts (`tools/gen_*.py`) return and print the actual saved path (so you'll see the uppercase filenames in the output).

Individual generators can also be run separately:

```bash
python3 tools/gen_background.py
python3 tools/gen_font.py
python3 tools/gen_sprite.py
```

### 4. Regenerate assets from PNG using png2asset (optional)

If you have GBDK-2020 installed and prefer `png2asset` for your workflow:

```bash
make convert
```

### 5. Clean build artifacts

```bash
make clean         # Remove build artifacts (obj/)
make clean-generated  # Remove generated PNG/C/H asset files in res/
make clean-all     # Run both clean and clean-generated
```

### 6. Check ROM bank usage

```bash
make romusage      # Show how ROM banks are utilized
```

See the **Autobanking** section below for details on ROM bank management.

### VS Code Tasks

Press **Ctrl+Shift+B** (or **Cmd+Shift+B**) to run the default **Build GBC ROM** task. Additional tasks are available via **Terminal → Run Task…**

---

## Autobanking

This project uses **GBDK-2020's autobanking** feature to automatically distribute code and data across multiple ROM banks. The Game Boy Color supports bank switching to access more than 32KB of ROM, and autobanking makes this transparent to the developer.

### What is Autobanking?

The Game Boy has a fixed 16KB ROM Bank 0 (always mapped at `0x0000-0x3FFF`) and a switchable 16KB window at `0x4000-0x7FFF` that can access any of the additional ROM banks (1-255 for MBC5). Autobanking lets the linker automatically:

- Assign functions and data to appropriate banks
- Generate bank-switching trampolines for cross-bank calls
- Optimize bank usage to minimize wasted space

### Compiler Flags

The Makefile includes these autobanking-related flags:

```makefile
LCCFLAGS = -Wl-yt0x1B -Wm-yc -Wl-j -Wl-yoA -Wm-ya4 -autobank -Wb-ext=.rel -Wb-v
```

- `-autobank` — Enable automatic bank assignment
- `-Wl-yoA` — Automatic bank area sizing (linker adjusts dynamically)
- `-Wm-ya4` — Autobanking allocator heuristic (balance bank filling)
- `-Wb-ext=.rel` — Use `.rel` extension for bank/relocation files
- `-Wb-v` — Verbose banker output (shows bank assignments during build)

### Autobanking Conventions

Files that should be autobanked (placed in switchable ROM banks) must include this directive at the top:

```c
#pragma bank 255
```

**Bank 255 is a special marker** that tells the linker "this file should be autobanked." The linker will assign it to an actual bank (1, 2, 3, etc.) automatically—it does NOT place the file literally in bank 255.

#### Required Header for Autobanked Files

```c
#pragma bank 255
#include <gbdk/platform.h>
#include <stdint.h>
```

The `<gbdk/platform.h>` header provides the `BANKREF()` and `BANKREF_EXTERN()` macros needed for proper bank reference tracking.

#### Marking Exported Symbols

For each exported function or const data array in an autobanked `.c` file, add `BANKREF()`:

```c
#pragma bank 255
#include <gbdk/platform.h>

BANKREF(level_data)
const uint8_t level_data[] = { /* ... */ };

BANKREF(update_level)
void update_level(void) BANKED {
    // ...
}
```

In the matching `.h` file, use `BANKREF_EXTERN()` and mark functions with `BANKED`:

```c
#pragma once
#include <gbdk/platform.h>

BANKREF_EXTERN(level_data)
extern const uint8_t level_data[];

BANKREF_EXTERN(update_level)
void update_level(void) BANKED;
```

The `BANKED` attribute tells the compiler this function lives in a switchable bank and may require bank-switching logic when called.

### What Should Be Autobanked?

**Files that SHOULD be autobanked (`#pragma bank 255`):**
- Game state implementations (`state_*.c`)
- Game-specific sprite logic (`sprite_player.c`, `sprite_enemy.c`)
- Large const data (backgrounds, sprites, fonts, sound data)
- Level data, map data, dialogue text

**Files that MUST stay in Bank 0 (NO autobanking):**
- `main.c` — Entry point and main loop
- Interrupt service routines (ISRs)
- Core engine utility code in `src/lib/src/` (unless specifically needed in banked code)

### Adding New Game States

When creating a new game state (e.g., `state_shop.c`):

1. **Create the `.c` file with autobanking directives:**

```c
#pragma bank 255
#include <gbdk/platform.h>
#include <gb/gb.h>
#include "states.h"

static void shop_init(void) {
    // Initialize shop screen
}

static void shop_update(void) {
    // Handle shop logic
}

static void shop_cleanup(void) {
    // Clean up shop resources
}

BANKREF(state_shop)
GameState state_shop = {
    shop_init,
    shop_update,
    shop_cleanup
};
```

2. **Create the matching `.h` file:**

```c
#pragma once
#include <gbdk/platform.h>
#include "states.h"

BANKREF_EXTERN(state_shop)
extern GameState state_shop;
```

3. **Register the state in `state_machine.c`** and rebuild.

### Adding New Sprite Classes

When creating a new sprite type (e.g., `sprite_boss.c`):

1. **Create the `.c` file with autobanking:**

```c
#pragma bank 255
#include <gbdk/platform.h>
#include <gb/gb.h>
#include "sprite.h"

static Sprite boss_sprite;

BANKREF(boss_init)
void boss_init(void) BANKED {
    // Initialize boss sprite
}

BANKREF(boss_update)
void boss_update(void) BANKED {
    // Update boss logic
}

BANKREF(boss_get_sprite)
Sprite* boss_get_sprite(void) BANKED {
    return &boss_sprite;
}
```

2. **Create the matching `.h` file:**

```c
#pragma once
#include <gbdk/platform.h>
#include "sprite.h"

BANKREF_EXTERN(boss_init)
void boss_init(void) BANKED;

BANKREF_EXTERN(boss_update)
void boss_update(void) BANKED;

BANKREF_EXTERN(boss_get_sprite)
Sprite* boss_get_sprite(void) BANKED;
```

3. **Include and call from your game state** — BANKED functions can be called directly; the linker handles bank switching automatically.

### Calling Banked Functions

BANKED functions can typically be called directly:

```c
boss_init();
boss_update();
```

The linker generates trampolines for cross-bank calls. For manual bank switching (advanced use):

```c
SWITCH_ROM(BANK(boss_update));
boss_update();
```

### Asset Generation with Autobanking

The asset generation tools (`tools/gbc_asset_builder.py`) automatically generate `.c` and `.h` files with autobanking conventions. Generated assets include:
- `#pragma bank 255` at the top of `.c` files
- `BANKREF()` macros for tile and palette data
- `BANKREF_EXTERN()` in headers

When you run `make generate`, all backgrounds, fonts, and sprites are created with proper autobanking markup.

### Checking ROM Usage

After building, check bank utilization:

```bash
make romusage
```

This shows how much space is used in each ROM bank. Example output:

```
Bank         Range                Size     Used  Used%     Free  Free%
--------     ----------------  -------  -------  -----  -------  -----
ROM_0        0x0000 -> 0x3FFF    16384     3184    19%    13200    81%
ROM_1        0x4000 -> 0x7FFF    16384    11549    70%     4835    30%
```

As your game grows, the linker will automatically allocate additional banks (ROM_2, ROM_3, etc.) as needed.

### Summary

- **Always use** `#pragma bank 255` for game states, sprites, and large data
- **Always mark** exported symbols with `BANKREF()` in `.c` and `BANKREF_EXTERN()` in `.h`
- **Always mark** banked functions with the `BANKED` attribute in headers
- **Keep** `main.c` and ISRs in Bank 0 (no `#pragma bank 255`)
- **Run** `make romusage` to monitor bank usage

See [`.github/copilot-instructions.md`](.github/copilot-instructions.md) for the complete autobanking workflow rules.

---

## GBC Color Features

### Palettes

GBC supports up to 8 background palettes and 8 sprite palettes, each with 4 colors (15-bit RGB, `RGB8` macro):

```c
/* background.c - 2 palettes × 4 colors */
const palette_color_t background_palettes[8] = {
    /* Palette 0: Sky */
    RGB8(155,200,234), RGB8(255,255,255), RGB8(52,136,52), RGB8(120,80,40),
    /* Palette 1: Ground */
    RGB8(52,136,52),   RGB8(80,160,80),  RGB8(120,80,40), RGB8(80,50,20)
};
```

### VRAM Bank 1 Tile Attributes

VRAM Bank 1 stores the per-tile attribute bytes (palette index, flip flags,
priority and the VRAM-bank select bit). These attribute bytes live in the
background attribute map (written with `VBK_REG = 1`) — they are not the
tile PATTERN data written with `set_bkg_data()`.

Write attribute bytes like this:

```c
VBK_REG = 1; /* switch to attribute bank */
set_bkg_tile_xy(x, y, attr_byte); /* attr_byte encodes palette/index/flags */
VBK_REG = 0; /* switch back to tile bank */
```

Note: the attribute byte also contains a bit that selects the alternate VRAM
bank for the tile pattern. If you place tile PATTERN data in VRAM bank 1 but
only write palette indices (e.g. `0x00`, `0x01`, `0x02`) without setting the
VRAM-bank bit, the patterns in bank 1 will not be used. Use the attribute
byte intentionally when you need to select alternate pattern banks.

---

## State Machine

The game uses a simple three-state machine:

```
[ Title Screen ] ──START──▶ [ Gameplay ] ──reach flag──▶ [ Win       ] ──START──▶ [ Title Screen ]
                                  │                                                       ▲
                                  └── lives=0 or timer=0 ──▶ [ Game Over ] ──START──────┘
```

Each state implements three callbacks:

```c
typedef struct {
    void (*init)(void);     // Called once on state entry
    void (*update)(void);   // Called every frame (after vsync)
    void (*cleanup)(void);  // Called once on state exit
} GameState;
```

To add a new state:
1. Add a new entry to `GameStateID` in `states.h`
2. Create `state_newstate.c/.h` implementing `init`, `update`, `cleanup` (with autobanking directives—see **Autobanking** section)
3. Add `&state_newstate` to the `states[]` array in `state_machine.c`

---

## Asset Generation Tools

The `tools/` directory contains Python scripts (requires `pip install pillow`) for
generating all PNG and C/H source files from scratch.  No GBDK installation is needed.

### `gbc_asset_builder.py` — reusable library

| Function | Description |
|---|---|
| `pixels_to_2bpp(rows_8x8)` | Convert 8×8 pixel tile to 16 GBDK 2bpp bytes |
| `tiles_to_2bpp_bytes(tiles)` | Flatten a list of tiles to a byte list |
| `png_to_tiles(png_path)` | Load indexed PNG and extract 8×8 tile pixel data |
| `make_indexed_png(grid, palette, path)` | Create indexed PNG from pixel array |
| `write_background_files(...)` | Write background `.c` + `.h` from tile/map/palette data |
| `write_font_files(...)` | Write font `.c` + `.h` from tile/palette data |
| `write_sprite_files(...)` | Write sprite `.c` + `.h` for 8×16 sprite mode |

### Creating your own example

1. **New background** – copy `tools/gen_background.py`, edit `TILES`, `TILEMAP`,
   and `PALETTE_COLORS`, then run it.  The output goes to `res/` by default; change
   `out_dir` in `main()` to target a different directory.

2. **New font** – copy `tools/gen_font.py`, edit `FONT_BITMAPS` (one 8-byte row
   bitmap per ASCII character), adjust `PALETTE_COLORS`.

3. **New sprite** – copy `tools/gen_sprite.py`, edit `FRAMES_TOP` / `FRAMES_BOTTOM`
   (8×8 pixel arrays using the colour aliases `T Y B D`), adjust `PALETTE_COLORS`.

4. Import `gbc_asset_builder` in your own script to reuse the 2bpp conversion and
   file-writing helpers for any custom asset type.

---

## Extending the Template

### Add a new background tileset

The recommended workflow is to author a `definition.py` (no external tools needed):

1. Create `res/backgrounds/<name>/definition.py` defining `TILES`, `TILEMAP_FLAT`, `PALETTE_COLORS`, `ATTR_MAP`, `MAP_W`, `MAP_H`, and optionally `COLLISION_TILE_IDS` / `COLLISION_TILE_DOWN_IDS`.
2. Run `make generate` — this produces `res/<name>.png`, `res/<name>.c`, and `res/<name>.h`.
3. In your state's `init()`, call `set_bkg_data()` and `set_bkg_palette()` using the generated constants, and write the attr map via `VBK_REG = 1`.

Alternatively, if you have GBDK-2020 and prefer `png2asset`:

1. Create a 160×144 indexed PNG with ≤4 colours per palette and add it to `res/`.
2. Run `make convert` to generate the `.c/.h` pair.

### Add more sprites

1. If you prefer manual PNG edits:
    - Edit the sprite PNG in `res/` (for example `res/player.png`). For this
       template the sprite layout expects 8×8 tiles arranged so that each 16×16
       logical sprite uses two 8×8 tiles stacked (8×16 mode). For a 16×16
       character frame you typically place the left and right halves as two
       consecutive tiles.
    - Regenerate the C/H asset pair with either the Python generators or
       `png2asset` (see below), then rebuild.

2. Using the built-in definition workflow (recommended):
    - Each sprite has a small directory under `res/sprites/` (for example
       `res/sprites/player/`) that contains a `definition.py` describing
       frames, animation constants and palette information. To add or update a
       game sprite create or edit `res/sprites/<name>/definition.py` and adjust
       the frames and metadata (start indices, frames-per-animation, tile
       counts, palette indices) to match your artwork.
    - Run the asset generator to produce the `res/<name>.c` and `res/<name>.h`
       files. The simplest way is to run the master generator which picks up
       all definitions:

```bash
make generate
```

After generation you will find symbols in the header such as
`PLAYER_TILE_COUNT`, `PLAYER_TILES_PER_FRAME`, `PLAYER_ANIM_WALK_START`,
and arrays `player_tiles`, `player_palettes` that your code can use.
Update your game code to reference the generated constants (for
example pass the generated `PLAYER_TILE_COUNT` as the `tile_base` or
use `PLAYER_ANIM_*` constants when selecting frames).

3. Alternative: convert PNGs directly with `png2asset` (GBDK tool):
    - Edit `res/<name>.png` and run:

```bash
make convert
```

This uses `png2asset` to create `.c/.h` files under `res/` (Makefile's
`convert` target is configured for the included assets).

4. Rebuild and test:

```bash
make all
make run
```

5. Notes & tips:
    - Generated headers expose both tile data and animation constants — use
       those instead of hardcoding tile indexes to keep code resilient when
       tile ordering changes.
    - If you only changed a single sprite definition but want to regenerate
       everything quickly, `make generate` is the simplest option. If you have
       a custom workflow you can invoke `python tools/gen_sprite.py` directly
       (see that script for CLI options).
    - Keep the `definition.py` alongside your source art in `res/sprites/`
       so the source of truth for the sprite layout is versioned with the
       project.

### Add sound

GBDK-2020 provides `<gb/sound.h>` for DMG/GBC audio channels. See the [GBDK-2020 docs](https://gbdk-2020.github.io/gbdk-2020/docs/api/gb_2sound_8h.html).

---

## Links

- [GBDK-2020 GitHub](https://github.com/gbdk-2020/gbdk-2020)
- [GBDK-2020 API Docs](https://gbdk-2020.github.io/gbdk-2020/docs/api/index.html)
- [Pan Docs (GBC hardware reference)](https://gbdev.io/pandocs/)
- [Emulicious Emulator](https://emulicious.net)
- [GB Dev community](https://gbdev.io)

---

## License

See [LICENSE](LICENSE).
