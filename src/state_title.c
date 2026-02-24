#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_title.h"
#include "utils.h"
#include "../res/background.h"
#include "../res/font.h"

/* FONT_FIRST_TILE: VRAM tile index where the font starts (after background tiles) */
#define FONT_FIRST_TILE BACKGROUND_TILE_COUNT

static uint8_t flash_counter;
static uint8_t show_prompt;

static void title_init(void) {
    uint8_t x, y;
    flash_counter = 0;
    show_prompt = 1;

    /* Fill background with sky tile (tile 0) */
    for (y = 0; y < BACKGROUND_MAP_HEIGHT; y++) {
        for (x = 0; x < BACKGROUND_MAP_WIDTH; x++) {
            uint8_t tile = background_map[y * BACKGROUND_MAP_WIDTH + x];
            set_bkg_tile_xy(x, y, tile);
        }
    }

    /* Draw title text */
    draw_text(4, 6,  "GBC TEMPLATE", FONT_FIRST_TILE);
    draw_text(3, 7,  "GAME STARTER", FONT_FIRST_TILE);
    draw_text(3, 13, "PRESS START",  FONT_FIRST_TILE);
}

static void title_update(void) {
    /* Flash the PRESS START prompt */
    flash_counter++;
    if (flash_counter >= 30U) {
        flash_counter = 0;
        show_prompt ^= 1;
        if (show_prompt) {
            draw_text(3, 13, "PRESS START", FONT_FIRST_TILE);
        } else {
            draw_text(3, 13, "           ", FONT_FIRST_TILE);
        }
    }

    /* Check for START button */
    if (joypad() & J_START) {
        switch_state(STATE_GAMEPLAY);
    }
}

static void title_cleanup(void) {
    /* Nothing to clean up */
}

const GameState state_title = {
    title_init,
    title_update,
    title_cleanup
};
