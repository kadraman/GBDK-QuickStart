
# copilot-instructions.md

Autobanking Workflow Rules for GBDK‑2020 Projects
=================================================

These are the rules Copilot must follow when creating or modifying files in a project that uses **autobanking** with **GBDK‑2020**.

Autobanking in GBDK works by marking any source file you want to place in a switchable ROM bank with:

```c
#pragma bank 255
```

Bank 255 enables autobanking for that file (it does **not** force the file into bank 255).

---

## 1. When creating or updating source files

### Files that should be autobanked must include:
```c
#pragma bank 255
```

### Files that must stay in Bank 0 (no autobanking):
- main.c
- ISR handlers
- global engine utility code

---

## 2. BANKREF Rules

### In the autobanked `.c` file:
```c
BANKREF(level_data)
const uint8_t level_data[] = { ... };

BANKREF(update_level)
void update_level(void) BANKED { ... }
```

### In the matching `.h` file:
```c
BANKREF_EXTERN(level_data)
BANKREF_EXTERN(update_level)

extern const uint8_t level_data[];
void update_level(void) BANKED;
```

---

## 3. Calling banked functions

### BANKED functions may be called directly:
```c
update_level();
```

### Or explicitly with `SWITCH_ROM`:
```c
SWITCH_ROM(BANK(update_level));
update_level();
```

---

## 4. Rules for ROM assets

- Large assets (maps, tiles, sprites, sound) must be:
  - placed in `#pragma bank 255` files
  - `const`
  - given a `BANKREF()`
- Never put large const data in Bank 0.

---

## 5. File layout recommendations

Each module should have:

```
level_X.c  → autobanked file with #pragma bank 255
level_X.h  → header with BANKREF_EXTERN declarations
```

---

## 6. Build flags

The build system must include:
- `-autobank`
- `-Wl-yoA` (automatic bank sizing)
- correct MBC type, e.g. `-Wl-yt0x1B`

---

## 7. Template for autobanked .c files

```c
#pragma bank 255
#include <gbdk/platform.h>
#include <stdint.h>

BANKREF(identifier)
const uint8_t identifier[] = { /* data */ };

BANKREF(func_name)
void func_name(void) BANKED {
    // ...
}
```

---

## 8. Template for autobanked .h files

```c
#ifndef MODULE_NAME_H
#define MODULE_NAME_H

#include <gbdk/platform.h>

BANKREF_EXTERN(identifier)
BANKREF_EXTERN(func_name)

extern const uint8_t identifier[];
void func_name(void) BANKED;

#endif

---

## 9. Checklist

- [ ] Add `#pragma bank 255` to each autobanked file
- [ ] Add `BANKREF()` to exported symbols
- [ ] Add `BANKREF_EXTERN()` in headers
- [ ] Mark functions `BANKED`
- [ ] Keep Bank 0 minimal
- [ ] Do not assign fixed bank numbers when using autobank

---

## 10. Project structure

This GBDK-QuickStart project follows a specific directory layout:

```
src/
├── lib/                       # Reusable library code (stays in Bank 0)
│   ├── include/               # Public headers
│   │   ├── sprite.h
│   │   ├── sprite_manager.h
│   │   ├── states.h
│   │   └── utils.h
│   └── src/                   # Library implementations
│       ├── sprite.c
│       ├── sprite_manager.c
│       ├── state_machine.c
│       └── utils.c
└── game/                      # Game-specific code (autobanked)
    ├── main.c                 # Entry point (Bank 0 - NO autobanking)
    ├── states/                # Game state implementations (autobanked)
    │   ├── state_title.c/.h
    │   ├── state_gameplay.c/.h
    │   ├── state_gameover.c/.h
    │   └── state_win.c/.h
    └── sprites/               # Sprite logic modules (autobanked)
        ├── sprite_player.c/.h
        └── sprite_enemy.c/.h

res/                           # Generated assets (autobanked)
├── backgrounds/               # Background definitions
│   ├── gameplay/definition.py
│   ├── title/definition.py
│   ├── gameover/definition.py
│   └── win/definition.py
├── fonts/
│   └── default/definition.py
├── sprites/
│   ├── player/definition.py
│   └── enemy/definition.py
└── [generated .c/.h/.png files]

tools/                         # Asset generation scripts
├── gbc_asset_builder.py      # Core asset generation library
├── gen_background.py
├── gen_font.py
├── gen_sprite.py
└── generate_assets.py        # Master generator (all assets)
```

### What goes where?

- **`src/lib/`** - Core reusable engine code (collision, sprite manager, state machine, utils). Stays in Bank 0.
- **`src/game/main.c`** - Entry point and main loop. Must stay in Bank 0 (NO `#pragma bank 255`).
- **`src/game/states/`** - Game state implementations. Each state *must* use autobanking.
- **`src/game/sprites/`** - Game-specific sprite logic. Each sprite module *must* use autobanking.
- **`res/`** - Generated asset files (.c/.h/.png). Generated assets automatically include autobanking.
- **`tools/`** - Python asset generators. Do not require GBDK installation.

---

## 11. Adding new game states

Game states implement the `GameState` interface (`init`, `update`, `cleanup` callbacks).

### Step-by-step:

1. **Create definition directory** (if using background assets):
   ```
   res/backgrounds/shop/definition.py
   ```
   Define `TILES`, `TILEMAP_FLAT`, `PALETTE_COLORS`, `ATTR_MAP`, `MAP_W`, `MAP_H`.

2. **Generate assets**:
   ```bash
   make generate
   ```
   This creates `res/bg_shop.c`, `res/bg_shop.h`, `res/bg_shop.png`.

3. **Create state implementation** in `src/game/states/state_shop.c`:
   ```c
   #pragma bank 255
   #include <gbdk/platform.h>
   #include <gb/gb.h>
   #include "states.h"
   #include "bg_shop.h"
   
   static void shop_init(void) {
       // Load background tiles and palettes
       set_bkg_data(0, BG_SHOP_TILE_COUNT, bg_shop_tiles);
       set_bkg_palette(0, BG_SHOP_PALETTE_COUNT, bg_shop_palettes);
       
       // Write tilemap
       VBK_REG = 0;
       set_bkg_tiles(0, 0, BG_SHOP_WIDTH, BG_SHOP_HEIGHT, bg_shop_map);
       
       // Write attributes (palette indices)
       VBK_REG = 1;
       set_bkg_tiles(0, 0, BG_SHOP_WIDTH, BG_SHOP_HEIGHT, bg_shop_attr);
       VBK_REG = 0;
       
       SHOW_BKG;
   }
   
   static void shop_update(void) {
       // Shop logic here
   }
   
   static void shop_cleanup(void) {
       // Cleanup resources
   }
   
   BANKREF(state_shop)
   const GameState state_shop = {
       shop_init,
       shop_update,
       shop_cleanup
   };
   ```

4. **Create header** in `src/game/states/state_shop.h`:
   ```c
   #ifndef STATE_SHOP_H
   #define STATE_SHOP_H
   
   #include <gbdk/platform.h>
   #include "states.h"
   
   BANKREF_EXTERN(state_shop)
   extern const GameState state_shop;
   
   #endif

5. **Register in state machine**:
   - Add `STATE_SHOP` to `GameStateID` enum in `src/lib/include/states.h`
   - Add `&state_shop` to `states[]` array in `src/lib/src/state_machine.c`
   - Include `"state_shop.h"` in `state_machine.c`

6. **Rebuild**:
   ```bash
   make clean
   make all
   ```

### Examples in this project:

- `src/game/states/state_title.c/.h` - Title screen with night sky background
- `src/game/states/state_gameplay.c/.h` - Main gameplay with scrolling level
- `src/game/states/state_gameover.c/.h` - Game over screen
- `src/game/states/state_win.c/.h` - Victory screen

---

## 12. Adding new sprite classes

Sprite classes encapsulate sprite logic (init, update, collision, animation).

### Step-by-step:

1. **Create sprite definition** in `res/sprites/boss/definition.py`:
   ```python
   # Define FRAMES_TOP, FRAMES_BOTTOM (8x8 tile pixel data)
   # Define PALETTE_COLORS
   # Define animation constants
   ```

2. **Generate sprite assets**:
   ```bash
   make generate
   ```
   This creates `res/boss.c`, `res/boss.h`, `res/boss.png` with proper autobanking.

3. **Create sprite logic** in `src/game/sprites/sprite_boss.c`:
   ```c
   #pragma bank 255
   #include <gbdk/platform.h>
   #include <gb/gb.h>
   #include "sprite.h"
   #include "sprite_manager.h"
   #include "boss.h"
   
   static Sprite boss_sprite;
   
   BANKREF(boss_init)
   void boss_init(uint8_t x, uint8_t y) BANKED {
       sprite_init(&boss_sprite, x, y, 
                   BOSS_TILE_BASE, BOSS_TILES_PER_FRAME,
                   BOSS_ANIM_IDLE_START, BOSS_ANIM_IDLE_FRAMES);
       sprite_manager_add(&boss_sprite);
   }
   
   BANKREF(boss_update)
   void boss_update(void) BANKED {
       // Boss AI logic
       sprite_update_oam(&boss_sprite);
   }
   
   BANKREF(boss_cleanup)
   void boss_cleanup(void) BANKED {
       sprite_manager_remove(&boss_sprite);
   }
   
   BANKREF(boss_get_sprite)
   Sprite* boss_get_sprite(void) BANKED {
       return &boss_sprite;
   }
   ```

4. **Create header** in `src/game/sprites/sprite_boss.h`:
   ```c
   #ifndef SPRITE_BOSS_H
   #define SPRITE_BOSS_H
   
   #include <gbdk/platform.h>
   #include "sprite.h"
   
   BANKREF_EXTERN(boss_init)
   void boss_init(uint8_t x, uint8_t y) BANKED;
   
   BANKREF_EXTERN(boss_update)
   void boss_update(void) BANKED;
   
   BANKREF_EXTERN(boss_cleanup)
   void boss_cleanup(void) BANKED;
   
   BANKREF_EXTERN(boss_get_sprite)
   Sprite* boss_get_sprite(void) BANKED;
   
   #endif

5. **Load sprite tiles** in your game state's `init()`:
   ```c
   set_sprite_data(BOSS_TILE_BASE, BOSS_TILE_COUNT, boss_tiles);
   set_sprite_palette(BOSS_PALETTE_INDEX, 1, boss_palettes);
   boss_init(80, 72);
   ```

6. **Call from game state's `update()`**:
   ```c
   boss_update();
   ```

7. **Rebuild**:
   ```bash
   make clean
   make all
   ```

### Examples in this project:

- `src/game/sprites/sprite_player.c/.h` - Player character with jump/walk animations
- `src/game/sprites/sprite_enemy.c/.h` - Patrolling enemy sprite

---

## 13. Asset generation workflow

Assets are generated using Python scripts in `tools/`. No GBDK installation required.

### Master generator (recommended):

```bash
make generate
# or: python tools/generate_assets.py
```

This auto-discovers all definitions in `res/backgrounds/`, `res/fonts/`, and `res/sprites/` and generates:
- `.png` files (indexed PNG images)
- `.c` files (with `#pragma bank 255`, `BANKREF()`, tile/palette data)
- `.h` files (with `BANKREF_EXTERN()`, extern declarations, constants)

### Individual generators:

```bash
python tools/gen_background.py    # Generate backgrounds only
python tools/gen_font.py           # Generate fonts only
python tools/gen_sprite.py         # Generate sprites only
```

### Asset generation library:

`tools/gbc_asset_builder.py` provides reusable functions:
- `write_background_files()` - Generates autobanked background .c/.h
- `write_font_files()` - Generates autobanked font .c/.h
- `write_sprite_files()` - Generates autobanked sprite .c/.h (8x16 mode)
- `write_sprite_files_animated()` - For multi-frame animated sprites

**All generated assets automatically include autobanking conventions:**
- `#pragma bank 255` in .c files
- `#include <gbdk/platform.h>`
- `BANKREF()` for tile/palette arrays
- `BANKREF_EXTERN()` in headers

### Adding a new background:

1. Create `res/backgrounds/forest/definition.py`
2. Define `TILES`, `TILEMAP_FLAT`, `PALETTE_COLORS`, `ATTR_MAP`, `MAP_W`, `MAP_H`
3. Run `make generate`
4. Add to `GENERATED_ASSETS` in Makefile (for clean-generated target)
5. Use in game state: `#include "bg_forest.h"`

### Adding a new sprite:

1. Create `res/sprites/npc/definition.py`
2. Define frames (FRAMES_TOP/FRAMES_BOTTOM), palette, animation constants
3. Run `make generate`
4. Add to `GENERATED_ASSETS` in Makefile
5. Create sprite logic in `src/game/sprites/sprite_npc.c/.h`
6. Load tiles in game state: `set_sprite_data(NPC_TILE_BASE, NPC_TILE_COUNT, npc_tiles)`

---

## 14. Build system (Makefile targets)

```bash
make all              # Build ROM (default target)
make generate         # Generate all assets (PNG + C + H) from definitions
make convert          # Convert PNGs to C/H using png2asset (optional workflow)
make run              # Build and launch in Emulicious emulator
make romusage         # Show ROM bank usage statistics
make clean            # Remove build artifacts (obj/)
make clean-generated  # Remove generated assets in res/
make clean-all        # Run both clean and clean-generated
```

### Build flags in Makefile:

```makefile
LCCFLAGS = -Wl-yt0x1B -Wm-yc -Wl-j -Wl-yoA -Wm-ya4 -autobank -Wb-ext=.rel -Wb-v
```

- `-Wl-yt0x1B` - MBC5 cartridge type (0x1B)
- `-Wm-yc` - Game Boy Color mode
- `-Wl-j` - Joined ROM output
- `-Wl-yoA` - Automatic bank area sizing
- `-Wm-ya4` - Autobanking allocator heuristic
- `-autobank` - Enable autobanking
- `-Wb-ext=.rel` - Use .rel extension for bank files
- `-Wb-v` - Verbose banker output

### Include paths:

The Makefile sets these include paths automatically:
- `-Isrc/lib/include` - Library headers
- `-Isrc/game` - Game code
- `-Isrc/game/states` - State headers
- `-Isrc/game/sprites` - Sprite headers
- `-Ires` - Generated asset headers

This means you can use simple includes:
```c
#include "sprite.h"          // from src/lib/include/
#include "state_gameplay.h"  // from src/game/states/
#include "sprite_player.h"   // from src/game/sprites/
#include "bg_title.h"        // from res/
```

---

## 15. Checking bank usage

After building, check ROM bank utilization:

```bash
make romusage
```

Example output:
```
Bank         Range                Size     Used  Used%     Free  Free%
--------     ----------------  -------  -------  -----  -------  -----
ROM_0        0x0000 -> 0x3FFF    16384     3184    19%    13200    81%
ROM_1        0x4000 -> 0x7FFF    16384    11549    70%     4835    30%
```

During build, the `-Wb-v` (verbose banker) flag shows which files were assigned to which banks:

```
== Banks assigned: 1 -> 1 (allowed 1 -> 255). Max including fixed: 1) ==
Bank 1: Size=16384, Free=4782, Reserved=0
     Area  Size  Bank in->out  File in->out
   _CODE_  2065    255 ->   1  obj/game/sprites/sprite_player.o
   _CODE_  2059    255 ->   1  obj/bg_gameplay.o
   ...
```

As your game grows, the linker automatically allocates additional banks (ROM_2, ROM_3, etc.).

---

## 16. Common patterns

### Game state with background + sprites:

```c
#pragma bank 255
#include <gbdk/platform.h>
#include <gb/gb.h>
#include "states.h"
#include "bg_level1.h"
#include "sprite_player.h"
#include "sprite_enemy.h"

static void level1_init(void) {
    // Load background
    set_bkg_data(0, BG_LEVEL1_TILE_COUNT, bg_level1_tiles);
    set_bkg_palette(0, BG_LEVEL1_PALETTE_COUNT, bg_level1_palettes);
    
    VBK_REG = 0;
    set_bkg_tiles(0, 0, BG_LEVEL1_WIDTH, BG_LEVEL1_HEIGHT, bg_level1_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BG_LEVEL1_WIDTH, BG_LEVEL1_HEIGHT, bg_level1_attr);
    VBK_REG = 0;
    
    // Load sprite tiles
    set_sprite_data(PLAYER_TILE_BASE, PLAYER_TILE_COUNT, player_tiles);
    set_sprite_data(ENEMY_TILE_BASE, ENEMY_TILE_COUNT, enemy_tiles);
    
    // Load sprite palettes
    set_sprite_palette(0, 1, player_palettes);
    set_sprite_palette(1, 1, enemy_palettes);
    
    // Initialize sprites
    player_init(20, 100);
    enemy_init(120, 100);
    
    SHOW_BKG;
    SHOW_SPRITES;
}

static void level1_update(void) {
    player_update();
    enemy_update();
}

static void level1_cleanup(void) {
    player_cleanup();
    enemy_cleanup();
}

BANKREF(state_level1)
GameState state_level1 = {
    level1_init,
    level1_update,
    level1_cleanup
};
```

### Sprite with collision:

```c
#pragma bank 255
#include <gbdk/platform.h>
#include "sprite.h"
#include "sprite_manager.h"
#include "utils.h"

static Sprite collectible_sprite;

BANKREF(collectible_init)
void collectible_init(uint8_t x, uint8_t y) BANKED {
    sprite_init(&collectible_sprite, x, y, 
                COLLECTIBLE_TILE_BASE, COLLECTIBLE_TILES_PER_FRAME,
                0, 4);  // 4-frame animation
    sprite_manager_add(&collectible_sprite);
}

BANKREF(collectible_check_collision)
uint8_t collectible_check_collision(Sprite* other) BANKED {
    return sprites_collide(&collectible_sprite, other);
}

BANKREF(collectible_collect)
void collectible_collect(void) BANKED {
    sprite_manager_remove(&collectible_sprite);
}
```

---

## 17. Summary for developers

When adding code to this project:

✅ **DO:**
- Use `#pragma bank 255` for game states, sprite modules, and large data
- Add `BANKREF()` / `BANKREF_EXTERN()` for all exported symbols
- Mark banked functions with `BANKED` attribute
- Keep `main.c` in Bank 0
- Use `make generate` to create assets with autobanking conventions
- Run `make romusage` to monitor bank usage
- Follow the project structure: states in `src/game/states/`, sprites in `src/game/sprites/`

❌ **DON'T:**
- Don't add `#pragma bank 255` to `main.c` or ISR handlers
- Don't put large const data in Bank 0
- Don't manually assign specific bank numbers (let autobanking handle it)
- Don't forget to add `#include <gbdk/platform.h>` in autobanked files
- Don't edit generated asset files manually (regenerate them instead)

📚 **References:**
- See `README.md` for detailed usage instructions
- See existing states in `src/game/states/` for examples
- See existing sprites in `src/game/sprites/` for examples
- See asset definitions in `res/backgrounds/`, `res/fonts/`, `res/sprites/`
