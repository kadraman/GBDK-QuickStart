#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_title.h"
#include "utils.h"
#include "../res/background.h"
#include "../res/font.h"

/* FONT_FIRST_TILE: VRAM tile index where the font starts */
#define FONT_FIRST_TILE BACKGROUND_TILE_COUNT

static uint8_t flash_counter;
static uint8_t show_prompt;

static void title_init(void)
{
    flash_counter = 0;
    show_prompt   = 1;

    /* Load background tilemap and palette attributes in one batch call */
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT-3,
                  background_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT-3,
                  background_attr_map);
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
