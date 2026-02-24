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
| Sprites | 8×16 mode, 8-frame walk cycle |
| Background | 20×18 tilemap with 2 GBC palettes |
| Font | 5×7 bitmap font, ASCII 32–127 |

---

## Prerequisites

| Tool | Purpose | Download |
|---|---|---|
| **GBDK-2020** | Compiler & linker (`lcc`) | https://github.com/gbdk-2020/gbdk-2020/releases |
| **Emulicious** | Emulator for testing | https://emulicious.net |
| **Python 3 + Pillow** | Asset regeneration script | `pip install pillow` |

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
│   └── state_gameover.c/.h # Game over state
├── res/
│   ├── background.png      # 160×144 indexed PNG (4-color sky/cloud/grass/ground)
│   ├── font.png            # 128×48 indexed PNG (96 chars, ASCII 32–127)
│   ├── sprite.png          # 32×64 indexed PNG (8 animation frames, 16×16 each)
│   ├── background.c/.h     # Pre-generated 2bpp tile data + GBC palettes + tilemap
│   ├── font.c/.h           # Pre-generated 2bpp font tile data
│   └── sprite.c/.h         # Pre-generated 2bpp sprite tile data + GBC palette
├── .vscode/
│   ├── c_cpp_properties.json  # IntelliSense paths for GBDK headers
│   ├── tasks.json             # Build / convert / clean tasks
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

### 3. Regenerate assets from PNG (optional)

If you edit the PNG files, regenerate the C/H files using `png2asset` via the Makefile:

```bash
make convert
```

### 4. Clean build artifacts

```bash
make clean
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

## Extending the Template

### Add a new background tileset

1. Create a 160×144 indexed PNG with ≤4 colors per palette
2. Add it to `res/` and run `make convert` or the Python generator
3. Call `set_bkg_data()` and `set_bkg_palette()` in `main.c`

### Add more sprites

1. Add 16×16 frames to `sprite.png` (each row of 2 tiles = one 8×16 sprite pair)
2. Update `SPRITE_FRAME_COUNT` and `SPRITE_TILE_COUNT` in `sprite.h`
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
