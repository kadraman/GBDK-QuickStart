#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_gameover.h"
#include "utils.h"
#include "bg_gameover.h"
#include "font.h"

/* Font palette with dark-crimson background colour to match bg_gameover */
static const palette_color_t gameover_font_palette[4] = {
    RGB8( 40,   0,   0),   /* 0 - dark crimson background     */
    RGB8(255, 255, 255),   /* 1 - white text                  */
    RGB8(170, 170, 170),   /* 2 - unused                      */
    RGB8( 85,  85,  85),   /* 3 - unused                      */
};

/* Font starts immediately after bg_gameover tiles in VRAM */
#define FONT_FIRST_TILE  BG_GAMEOVER_TILE_COUNT

static uint8_t prev_joy;

static void gameover_init(void)
{
    uint8_t save_bank;
    prev_joy = 0;

    /* Switch to asset bank before loading ROM data into VRAM/palettes.
     * Save _current_bank so the restore is correct regardless of which
     * bank this function is executing from. */
    save_bank = _current_bank;
    SWITCH_ROM(BANK(bg_gameover_tiles));

    /* Load game-over background tiles into VRAM slot 0 */
    set_bkg_data(0, BG_GAMEOVER_TILE_COUNT, bg_gameover_tiles);
    /* Font tiles immediately after background tiles */
    set_bkg_data(BG_GAMEOVER_TILE_COUNT, FONT_TILE_COUNT, font_tiles);

    /* Set game-over background palettes (slots 0-1) */
    set_bkg_palette(0, BG_GAMEOVER_PALETTE_COUNT, bg_gameover_palettes);
    /* Font palette with dark-crimson background (slot 2) */
    set_bkg_palette(2, 1, gameover_font_palette);

    /* Load tilemap and palette attributes; reset scroll */
    set_bkg_tiles(0, 0, BG_GAMEOVER_MAP_WIDTH, BG_GAMEOVER_MAP_HEIGHT,
                  bg_gameover_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BG_GAMEOVER_MAP_WIDTH, BG_GAMEOVER_MAP_HEIGHT,
                  bg_gameover_attr_map);
    VBK_REG = 0;

    SWITCH_ROM(save_bank);

    SCX_REG = 0;
    SCY_REG = 0;

    HIDE_WIN;  /* hide the HUD window shown during gameplay */

    draw_text(5,  6, "GAME OVER",   FONT_FIRST_TILE);
    draw_text(2,  9, "PRESS START", FONT_FIRST_TILE);
}

static void gameover_update(void)
{
    uint8_t joy       = joypad();
    uint8_t joy_press = (uint8_t)(joy & ~prev_joy);

    if (joy_press & J_START) {
        switch_state(STATE_TITLE_SCREEN);
    }
    prev_joy = joy;
}

static void gameover_cleanup(void)
{
    /* Nothing to clean up */
}

const GameState state_gameover = {
    gameover_init,
    gameover_update,
    gameover_cleanup
};
