#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_gameover.h"
#include "utils.h"
#include "../res/background.h"
#include "../res/font.h"

#define FONT_FIRST_TILE BACKGROUND_TILE_COUNT

static uint8_t prev_joy;

static void gameover_init(void) {
    uint8_t x, y;
    prev_joy = 0;

    /* Clear background to sky tile */
    for (y = 0; y < BACKGROUND_MAP_HEIGHT; y++) {
        for (x = 0; x < BACKGROUND_MAP_WIDTH; x++) {
            set_bkg_tile_xy(x, y, 0);
        }
    }

    /* Reset all tile attributes to palette 0 (sky) */
    VBK_REG = 1;
    for (y = 0; y < BACKGROUND_MAP_HEIGHT; y++) {
        for (x = 0; x < BACKGROUND_MAP_WIDTH; x++) {
            set_bkg_tile_xy(x, y, 0x00U);
        }
    }
    VBK_REG = 0;

    draw_text(5, 7,  "GAME OVER",   FONT_FIRST_TILE);
    draw_text(3, 10, "SCORE: 000",  FONT_FIRST_TILE);
    draw_text(2, 14, "PRESS START", FONT_FIRST_TILE);
}

static void gameover_update(void) {
    uint8_t joy = joypad();
    uint8_t joy_pressed = joy & ~prev_joy;

    if (joy_pressed & J_START) {
        switch_state(STATE_TITLE_SCREEN);
    }

    prev_joy = joy;
}

static void gameover_cleanup(void) {
    /* Nothing to clean up */
}

const GameState state_gameover = {
    gameover_init,
    gameover_update,
    gameover_cleanup
};
