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
- **Timer HUD**: A 60-second countdown (`TIME: XX`) displayed in the HUD during gameplay; reaching zero triggers game-over.
- **Wide pitfall level**: 48-tile (384 px) scrolling level with 3 pit zones, 4 raised platforms, and column streaming into the 32-tile hardware ring buffer.
- **Asset tooling**: Python generators in `tools/` to produce indexed PNGs and `.c/.h` asset files; optional `png2asset` conversion via Makefile.
- **Modular includes**: Makefile adds `-Isrc/lib/include` and `-Ires` so code can `#include "sprite.h"` and `#include "background.h"` without path noise.

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
│   │   │   └── states.h
│   │   └── src/              # Library implementations
│   │       ├── sprite.c
│   │       ├── sprite_manager.c
│   │       └── state_machine.c
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
│   │   ├── gameplay/definition.py  → background.c/.h (48-tile wide level)
│   │   ├── title/definition.py     → bg_title.c/.h   (night sky)
│   │   ├── gameover/definition.py  → bg_gameover.c/.h (crimson sky)
│   │   └── win/definition.py       → bg_win.c/.h     (golden sky)
│   ├── fonts/
│   │   └── default/definition.py  → font.c/.h
│   ├── sprites/
│   │   ├── player/definition.py   → player.c/.h (16x16 animated)
│   │   └── enemy/definition.py    → enemy.c/.h  (8x8 patrol enemy)
│   ├── background.png / background.c/.h
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
   `#include "sprite.h"` or `#include "background.h"` as seen in the code.
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
make clean
# Remove generated PNG/C/H asset files in `res/`:
make clean-generated
```

### VS Code Tasks

Press **Ctrl+Shift+B** (or **Cmd+Shift+B**) to run the default **Build GBC ROM** task. Additional tasks are available via **Terminal → Run Task…**

---

## ROM Banking

### Overview

The project uses **GBDK-2020 auto-banking with MBC5**, enabled by the
following Makefile flags:

```makefile
LCCFLAGS += -Wm-yt25   # MBC5 cartridge type (up to 512 × 16 KB banks)
LCCFLAGS += -Wm-ybo    # auto bank overflow: linker fills each bank in turn
```

**How auto-banking works:**
- ROM bank 0 (0x0000–0x3FFF) is fixed and always visible. It holds the GBDK
  library runtime and interrupt vectors.
- ROM bank 1 (0x4000–0x7FFF) is the default switchable bank. Game code and
  asset data are placed there initially.
- When bank 1 becomes full, `-Wm-ybo` causes the linker to automatically
  overflow surplus data into bank 2, then bank 3, and so on.
- No manual linker scripts or bank-number calculations are required.

### Marking asset data as banked

Every `res/` asset source file is decorated with two things:

```c
/* at the top of the file, before the include */
#pragma bank 1          /* pin this file's data to ROM bank 1 */
#include "bg_title.h"

/* immediately after the tile data array */
const uint8_t bg_title_tiles[160] = { ... };
BANKREF(bg_title_tiles) /* let BANK() resolve the actual bank at link time */
```

The `BANKREF(symbol)` macro creates an internal reference variable that the
`BANK(symbol)` macro reads at runtime to obtain the bank number the symbol
was placed in. This means even if auto-banking overflows data to bank 2 or
higher, the access code needs no manual updates.

The matching header declares:

```c
BANKREF_EXTERN(bg_title_tiles)   /* make the bank reference visible */
extern const uint8_t bg_title_tiles[];
```

### Accessing banked data from game code

Before calling any GBDK function that dereferences a pointer into the banked
area (e.g. `set_bkg_data`, `set_sprite_data`, `set_bkg_palette`,
`set_bkg_tiles`), save the current bank, switch to the asset bank, and restore
the saved bank afterwards:

```c
uint8_t save_bank = _current_bank;   /* save caller's bank */
SWITCH_ROM(BANK(bg_title_tiles));    /* switch to asset bank */

set_bkg_data(0, BG_TITLE_TILE_COUNT, bg_title_tiles);
set_bkg_data(BG_TITLE_TILE_COUNT, FONT_TILE_COUNT, font_tiles);
set_bkg_palette(0, BG_TITLE_PALETTE_COUNT, bg_title_palettes);

SWITCH_ROM(save_bank);               /* restore caller's bank */
```

Saving `_current_bank` (the GBDK-maintained variable tracking the active bank)
and restoring it is safer than hardcoding `SWITCH_ROM(1)`, because it stays
correct if the calling code itself ever overflows to a higher bank.

All state `init()` functions and `main()` already follow this pattern.  The
streaming helper `load_bg_column()` in `state_gameplay.c` also performs its
own save/restore so it is safe to call from any bank context.

### What "auto-banking" means — and what it does not

**The linker is automatic; runtime bank switching is not.**

- `#pragma bank 1` and `-Wm-ybo` tell the *linker* to assign asset data to
  ROM bank 1, and to automatically overflow into bank 2, 3, … when a bank is
  full — you never need to edit a linker script or track bank sizes manually.

- However, the GBC hardware always executes code from a single visible bank
  at a time. Whenever your code needs to read data that lives in a *different*
  bank, it must explicitly call `SWITCH_ROM`. There is no CPU-level mechanism
  that makes this automatic.

- The `BANK(symbol)` macro resolves the correct bank number at link time, so
  `SWITCH_ROM(BANK(symbol))` is always correct even after overflow — no manual
  number tracking is required.

To summarise: *bank assignment* is automatic (linker), *bank switching* is
explicit (your code), but the `BANK()` macro means you never hardcode a number.

### Guidelines for adding new assets

1. **Create the asset files** (PNG → run `make generate` or the matching
   `tools/gen_*.py` script).  The generator scripts in `tools/` have been
   updated to emit `#pragma bank 1`, `BANKREF(symbol)`, and
   `BANKREF_EXTERN(symbol)` automatically.

2. **No manual bank number assignment needed** for new assets.  As long as the
   `.c` file begins with `#pragma bank 1` and ends with `BANKREF(name_tiles)`,
   and the `.h` file contains `BANKREF_EXTERN(name_tiles)`, the banking
   infrastructure handles the rest.

3. **When loading the asset** (usually in a state's `init()` function), wrap
   the GBDK calls with save/restore:
   ```c
   uint8_t save_bank = _current_bank;
   SWITCH_ROM(BANK(my_new_tiles));
   set_bkg_data(..., my_new_tiles);
   /* ... other loads from the same bank ... */
   SWITCH_ROM(save_bank);
   ```

4. **Watch bank 1 capacity (~16 KB / 0x4000 bytes)**. If you add many large
   assets or a significant amount of game code, bank 1 may fill up. When
   `-Wm-ybo` overflows data to bank 2, the `BANK(symbol)` macro returns 2
   automatically, so `SWITCH_ROM(BANK(symbol))` remains correct with no
   manual updates needed.  The `save_bank` / restore pattern means calling
   code does not need to know which bank it is executing from.

5. **Functions that read banked data every frame** (like `load_bg_column`)
   should call `SWITCH_ROM` internally rather than relying on the caller to do
   so, because they may be invoked from multiple call sites or different bank
   contexts in the future.

6. **Do not call banked functions directly from a different bank** without the
   `__banked` attribute and corresponding `BANKREF`/`BANK` indirection.
   Cross-bank function calls require the `__banked` attribute on the callee;
   see the [GBDK-2020 banked-functions docs](https://gbdk-2020.github.io/gbdk-2020/docs/api/docs_supported_consoles.html)
   for details.

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
[ Title Screen ] --START--> [ Gameplay ] --START--> [ Game Over ] --START--> [ Title Screen ]
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
2. Create `state_newstate.c/.h` implementing `init`, `update`, `cleanup`
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

1. Create a 160×144 indexed PNG with ≤4 colors per palette
2. Add it to `res/` and run `make convert` or the Python generator
3. Call `set_bkg_data()` and `set_bkg_palette()` in `main.c`

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
