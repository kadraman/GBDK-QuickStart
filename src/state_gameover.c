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

static void gameover_init(void)
{
    prev_joy = 0;

    /* Reload background tilemap and attributes; reset scroll */
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_attr_map);
    VBK_REG = 0;

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
