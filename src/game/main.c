#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "player.h"
#include "enemy.h"

/*
 * main.c – entry point for the GBDK-QuickStart GBC template.
 *
 * Responsibilities:
 *   - Load sprite tile data (player + enemy) into OBJ VRAM once.
 *   - Set up GBC sprite palettes (slots 0 and 1).
 *   - Set up shared HUD background palettes (slots 3 and 4).
 *   - Background tiles and font tiles are loaded per-state in each
 *     state's init() function (to support distinct per-state backgrounds).
 *
 * VRAM tile layout (per-state, loaded by each state's init):
 *   BKG slots 0 .. <bg_tile_count>-1 : background tiles for current state
 *   BKG slots <bg_tile_count> ..     : font tiles
 *
 * OBJ tile layout (loaded once here):
 *   Slots 0 .. PLAYER_TILE_COUNT-1              : player tiles
 *   Slots PLAYER_TILE_COUNT .. (P+E tile count) : enemy tiles
 */

/* HUD window palette (dark background, white text) */
static const palette_color_t hud_palette[4] = {
    RGB8( 10,  10,  40),   /* 0 - HUD background (dark navy) */
    RGB8(255, 255, 255),   /* 1 - white text                 */
    RGB8(200, 200, 200),   /* 2 - light grey                 */
    RGB8(150, 150, 150),   /* 3 - mid grey                   */
};

/* HUD red-text palette (dark background, red text – used for hearts) */
static const palette_color_t hud_red_palette[4] = {
    RGB8( 10,  10,  40),   /* 0 - HUD background             */
    RGB8(220,   0,   0),   /* 1 - red text (hearts)          */
    RGB8(255, 150, 150),   /* 2 - light red                  */
    RGB8(150,   0,   0),   /* 3 - dark red                   */
};

void main(void) {
    DISPLAY_OFF;

    /* --- GBC background palettes shared across all states --- */
    /* Slot 3: HUD palette (white text on dark background) */
    set_bkg_palette(3, 1, hud_palette);
    /* Slot 4: HUD red palette (red text on dark background – lives hearts) */
    set_bkg_palette(4, 1, hud_red_palette);

    /* --- Switch to asset bank and load sprite data ---
     * player_tiles and enemy_tiles are in bank 1 (#pragma bank 1).
     * BANK() resolves the correct bank at link time so this stays correct
     * even if auto-banking places these arrays in a higher bank later. */
    SWITCH_ROM(BANK(player_tiles));

    /* --- GBC sprite palettes --- */
    /* Slot 0: player palette */
    set_sprite_palette(0, PLAYER_PALETTE_COUNT, player_palettes);
    /* Slot 1: enemy palette */
    set_sprite_palette(1, ENEMY_PALETTE_COUNT, enemy_palettes);

    /* --- Load sprite tiles (persists across all states) --- */
    set_sprite_data(0, PLAYER_TILE_COUNT, player_tiles);
    set_sprite_data(PLAYER_TILE_COUNT, ENEMY_TILE_COUNT, enemy_tiles);

    /* Restore game code bank */
    SWITCH_ROM(1);

    /* Use 8x16 sprite mode */
    SPRITES_8x16;

    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    /* Start with the title screen */
    switch_state(STATE_TITLE_SCREEN);

    /* Main game loop */
    while (1) {
        vsync();
        run_current_state();
    }
}
