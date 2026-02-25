#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "../res/background.h"
#include "../res/font.h"
#include "../res/player.h"

/* HUD window palette (dark background, white text) */
static const palette_color_t hud_palette[4] = {
    RGB8( 10,  10,  40),   /* 0 - HUD background (dark navy) */
    RGB8(255, 255, 255),   /* 1 - white text                 */
    RGB8(200, 200, 200),   /* 2 - light grey                 */
    RGB8(150, 150, 150),   /* 3 - mid grey                   */
};

/* HUD red-text palette (dark background, red text — used for hearts) */
static const palette_color_t hud_red_palette[4] = {
    RGB8( 10,  10,  40),   /* 0 - HUD background             */
    RGB8(220,   0,   0),   /* 1 - red text (hearts)          */
    RGB8(255, 150, 150),   /* 2 - light red                  */
    RGB8(150,   0,   0),   /* 3 - dark red                   */
};

void main(void) {
    DISPLAY_OFF;

    /* --- Load tile graphics into VRAM bank 0 --- */
    /* Background tiles at slots 0 .. BACKGROUND_TILE_COUNT-1 */
    set_bkg_data(0, BACKGROUND_TILE_COUNT, background_tiles);
    /* Font tiles immediately after background tiles */
    set_bkg_data(BACKGROUND_TILE_COUNT, FONT_TILE_COUNT, font_tiles);

    /* --- GBC background palettes (slots 0-1: sky and ground) --- */
    set_bkg_palette(0, BACKGROUND_PALETTE_COUNT, background_palettes);
    /* Slot 2: font palette (black text on sky-matching background) */
    set_bkg_palette(2, FONT_PALETTE_COUNT, font_palettes);
    /* Slot 3: HUD palette (white text on dark background) */
    set_bkg_palette(3, 1, hud_palette);
    /* Slot 4: HUD red palette (red text on dark background — lives hearts) */
    set_bkg_palette(4, 1, hud_red_palette);

    /* --- GBC sprite palette (slot 0) --- */
    set_sprite_palette(0, PLAYER_PALETTE_COUNT, player_palettes);

    /* --- Load sprite tiles --- */
    set_sprite_data(0, PLAYER_TILE_COUNT, player_tiles);

    /* Use 8x16 sprite mode (two stacked 8x8 OBJ tiles per sprite) */
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
