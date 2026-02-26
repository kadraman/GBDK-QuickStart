#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_title.h"
#include "utils.h"
#include "../res/bg_title.h"
#include "../res/font.h"

/* Font starts at VRAM tile 128 (fixed across all states) */
#define FONT_FIRST_TILE 128U

/* Font palette with night-sky background colour to match bg_title */
static const palette_color_t title_font_palette[4] = {
    RGB8(  0,   0,  60),   /* 0 - night sky background        */
    RGB8(255, 255, 180),   /* 1 - warm white text             */
    RGB8(170, 170, 170),   /* 2 - unused                      */
    RGB8( 85,  85,  85),   /* 3 - unused                      */
};

static uint8_t flash_counter;
static uint8_t show_prompt;

static void title_init(void)
{
    flash_counter = 0;
    show_prompt   = 1;

    /* Load title-screen background tiles into VRAM slot 0 */
    set_bkg_data(0, BG_TITLE_TILE_COUNT, bg_title_tiles);

    /* Set title background palettes (slots 0-1) */
    set_bkg_palette(0, BG_TITLE_PALETTE_COUNT, bg_title_palettes);

    /* Set font palette to match night-sky background (slot 2) */
    set_bkg_palette(2, 1, title_font_palette);

    /* Load tilemap and palette attributes */
    set_bkg_tiles(0, 0, BG_TITLE_MAP_WIDTH, BG_TITLE_MAP_HEIGHT, bg_title_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BG_TITLE_MAP_WIDTH, BG_TITLE_MAP_HEIGHT, bg_title_attr_map);
    VBK_REG = 0;

    SCX_REG = 0;
    SCY_REG = 0;

    /* Draw title text */
    draw_text(6, 3,  "GBDK-GBC", FONT_FIRST_TILE);
    draw_text(1, 4,  "QuickStart Template", FONT_FIRST_TILE);
    draw_text(4, 16, "PRESS START",  FONT_FIRST_TILE);
}

static void title_update(void)
{
    flash_counter++;
    if (flash_counter >= 30U) {
        flash_counter = 0;
        show_prompt ^= 1U;
        if (show_prompt) {
            draw_text(4, 16, "PRESS START", FONT_FIRST_TILE);
        } else {
            draw_text(4, 16, "           ", FONT_FIRST_TILE);
        }
    }

    if (joypad() & J_START) {
        switch_state(STATE_GAMEPLAY);
    }
}

static void title_cleanup(void)
{
    /* Nothing to clean up */
}

const GameState state_title = {
    title_init,
    title_update,
    title_cleanup
};
