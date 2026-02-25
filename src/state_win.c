#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_win.h"
#include "utils.h"
#include "../res/background.h"
#include "../res/font.h"

#define FONT_FIRST_TILE BACKGROUND_TILE_COUNT

static uint8_t prev_joy;

static void win_init(void)
{
    prev_joy = 0;

    /* Reload background tilemap and reset scroll */
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_attr_map);
    VBK_REG = 0;

    SCX_REG = 0;
    SCY_REG = 0;

    HIDE_WIN;  /* hide the HUD window shown during gameplay */

    draw_text(4,  5, "YOU WIN!",    FONT_FIRST_TILE);
    draw_text(1,  7, "CONGRATULATIONS!", FONT_FIRST_TILE);
    draw_text(2,  9, "PRESS START", FONT_FIRST_TILE);
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
