#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_win.h"
#include "utils.h"
#include "bg_win.h"
#include "font.h"

/* Font palette with golden-sky background colour to match bg_win */
static const palette_color_t win_font_palette[4] = {
    RGB8(255, 200,  50),   /* 0 - golden sky background       */
    RGB8(  0,   0,   0),   /* 1 - black text                  */
    RGB8(170, 170, 170),   /* 2 - unused                      */
    RGB8( 85,  85,  85),   /* 3 - unused                      */
};

/* Font starts immediately after bg_win tiles in VRAM */
#define FONT_FIRST_TILE  BG_WIN_TILE_COUNT

static uint8_t prev_joy;

static void win_init(void)
{
    prev_joy = 0;

    /* Switch to asset bank before loading ROM data into VRAM/palettes. */
    SWITCH_ROM(BANK(bg_win_tiles));

    /* Load win background tiles into VRAM slot 0 */
    set_bkg_data(0, BG_WIN_TILE_COUNT, bg_win_tiles);
    /* Font tiles immediately after background tiles */
    set_bkg_data(BG_WIN_TILE_COUNT, FONT_TILE_COUNT, font_tiles);

    /* Set win background palettes (slots 0-1) */
    set_bkg_palette(0, BG_WIN_PALETTE_COUNT, bg_win_palettes);
    /* Font palette with golden-sky background (slot 2) */
    set_bkg_palette(2, 1, win_font_palette);

    /* Load tilemap and palette attributes; reset scroll */
    set_bkg_tiles(0, 0, BG_WIN_MAP_WIDTH, BG_WIN_MAP_HEIGHT, bg_win_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BG_WIN_MAP_WIDTH, BG_WIN_MAP_HEIGHT, bg_win_attr_map);
    VBK_REG = 0;

    /* Restore game code bank */
    SWITCH_ROM(1);

    SCX_REG = 0;
    SCY_REG = 0;

    HIDE_WIN;  /* hide the HUD window shown during gameplay */

    draw_text(4,  5, "YOU WIN!",         FONT_FIRST_TILE);
    draw_text(1,  7, "CONGRATULATIONS!", FONT_FIRST_TILE);
    draw_text(2,  9, "PRESS START",      FONT_FIRST_TILE);
}

static void win_update(void)
{
    uint8_t joy       = joypad();
    uint8_t joy_press = (uint8_t)(joy & ~prev_joy);

    if (joy_press & J_START) {
        switch_state(STATE_TITLE_SCREEN);
    }
    prev_joy = joy;
}

static void win_cleanup(void)
{
    /* Nothing to clean up */
}

const GameState state_win = {
    win_init,
    win_update,
    win_cleanup
};
