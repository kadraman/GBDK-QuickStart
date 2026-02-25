# GBC-Template

A complete starter template for **GameBoy Color (GBC)** games, built with [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020). Includes a state-machine game loop, pre-generated 2bpp tile assets, GBC color palettes, animated sprites, and VS Code tooling.

---

## Overview

| Feature | Details |
|---|---|
| Hardware target | Game Boy Color (GBC) |
| SDK | GBDK-2020 (`lcc` / `sdcc`) |
| Language | C (C99 compatible) |
| Screens | Title → Gameplay → Game Over |
| Sprites | 8×16 mode, 4-frame walk cycle |
| Background | 20×18 tilemap with 2 GBC palettes |
| Font | 5×7 bitmap font, ASCII 32–127 |

---

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
GBC-Template/
├── src/
│   ├── main.c              # Entry point: VRAM setup, palette load, main loop
│   ├── states.h            # GameState struct + enum + function declarations
│   ├── state_machine.c     # switch_state() / run_current_state() implementation
│   ├── state_title.c/.h    # Title screen state
│   ├── state_gameplay.c/.h # Gameplay state (d-pad movement, animation)
│   ├── state_gameover.c/.h # Game over state
│   └── utils.c/.h          # Shared draw_text() helper
├── res/
│   ├── background.png      # 160×144 indexed PNG (sky/cloud/grass/ground, 32×18 tiles)
│   ├── font.png            # 128×56 indexed PNG (101 chars, ASCII 32–127 + ♠♣♥♦►)
│   ├── player.png          # 8×N  indexed PNG (player: idle/walk/jump/die animations)
│   ├── background.c/.h     # Pre-generated 2bpp tile data + GBC palettes + tilemap
│   ├── font.c/.h           # Pre-generated 2bpp font tile data
│   └── sprite.c/.h         # Pre-generated 2bpp sprite tile data + GBC palette
├── tools/
│   ├── gbc_asset_builder.py  # Reusable library: 2bpp conversion, .c/.h writers
│   ├── gen_background.py     # Generates res/background.{png,c,h}
│   ├── gen_font.py           # Generates res/font.{png,c,h}
│   ├── gen_sprite.py         # Generates res/sprite.{png,c,h}
│   └── generate_assets.py    # Master script: runs all three generators
├── .vscode/
│   ├── c_cpp_properties.json  # IntelliSense paths for GBDK headers
│   ├── tasks.json             # Build / generate / convert / clean tasks
│   └── extensions.json        # Recommended VS Code extensions
├── Makefile                # Build system
└── README.md
```

---

## Build Instructions

### 1. Build the ROM

```bash
make all
# Output: obj/GBCTemplate.gbc
```

### 2. Run in Emulicious

```bash
Emulicious obj/GBCTemplate.gbc
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

GBC uses VRAM Bank 1 to store per-tile attributes (palette index, flip flags, priority):

```c
VBK_REG = 1;  /* Switch to attribute bank */
set_bkg_tile_xy(x, y, palette_index);  /* 0x00 = palette 0, 0x01 = palette 1, … */
VBK_REG = 0;  /* Switch back to tile bank */
```

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

1. Add 16×16 frames to `sprite.png` (each row of 2 tiles = one 8×16 sprite pair)
2. Update `PLAYER_ANIM_WALK_FRAMES` in `player.h`
3. Use `set_sprite_tile()` / `move_sprite()` in your state

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
