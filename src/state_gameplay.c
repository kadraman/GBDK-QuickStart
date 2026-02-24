#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_gameplay.h"
#include "../res/background.h"
#include "../res/sprite.h"

/* Sprite position */
static uint8_t sprite_x;
static uint8_t sprite_y;
/* Animation */
static uint8_t anim_counter;
static uint8_t anim_frame;
/* Pause flag */
static uint8_t paused;
static uint8_t prev_joy;

static void gameplay_init(void) {
    uint8_t x, y;
    sprite_x = 80;
    sprite_y = 72;
    anim_counter = 0;
    anim_frame = 0;
    paused = 0;
    prev_joy = 0;

    /* Draw background tilemap */
    for (y = 0; y < BACKGROUND_MAP_HEIGHT; y++) {
        for (x = 0; x < BACKGROUND_MAP_WIDTH; x++) {
            uint8_t tile = background_map[y * BACKGROUND_MAP_WIDTH + x];
            set_bkg_tile_xy(x, y, tile);
        }
    }

    /* Set tile attributes in VRAM bank 1 (palette assignment) */
    VBK_REG = 1;
    for (y = 0; y < BACKGROUND_MAP_HEIGHT; y++) {
        for (x = 0; x < BACKGROUND_MAP_WIDTH; x++) {
            /* Top half uses palette 0 (sky colors), bottom half uses palette 1 (ground) */
            uint8_t attr = (y < 10) ? 0x00 : 0x01;
            set_bkg_tile_xy(x, y, attr);
        }
    }
    VBK_REG = 0;

    /* Place sprites (8x16 mode: 2 sprites stacked = one 16x16 character) */
    /* Sprite 0 = top half, Sprite 1 = bottom half */
    set_sprite_tile(0, (uint8_t)(anim_frame * SPRITE_TILES_PER_FRAME));
    set_sprite_tile(1, (uint8_t)(anim_frame * SPRITE_TILES_PER_FRAME + 2U));
    move_sprite(0, sprite_x, sprite_y);
    move_sprite(1, sprite_x, (uint8_t)(sprite_y + 16U));
}

static void gameplay_update(void) {
    uint8_t joy = joypad();
    uint8_t joy_pressed = joy & ~prev_joy;

    if (!paused) {
        /* Move sprite with d-pad */
        if (joy & J_RIGHT && sprite_x < 152U) sprite_x++;
        if (joy & J_LEFT  && sprite_x > 8U)   sprite_x--;
        if (joy & J_DOWN  && sprite_y < 128U)  sprite_y++;
        if (joy & J_UP    && sprite_y > 16U)   sprite_y--;

        /* Animate sprite */
        anim_counter++;
        if (anim_counter >= 10U) {
            anim_counter = 0;
            anim_frame = (uint8_t)((anim_frame + 1U) % SPRITE_FRAME_COUNT);
            set_sprite_tile(0, (uint8_t)(anim_frame * SPRITE_TILES_PER_FRAME));
            set_sprite_tile(1, (uint8_t)(anim_frame * SPRITE_TILES_PER_FRAME + 2U));
        }

        /* Update sprite position */
        move_sprite(0, sprite_x, sprite_y);
        move_sprite(1, sprite_x, (uint8_t)(sprite_y + 16U));
    }

    /* Toggle pause or go to game over on START */
    if (joy_pressed & J_START) {
        switch_state(STATE_GAME_OVER);
    }

    prev_joy = joy;
}

static void gameplay_cleanup(void) {
    /* Hide sprites */
    move_sprite(0, 0, 0);
    move_sprite(1, 0, 0);
}

const GameState state_gameplay = {
    gameplay_init,
    gameplay_update,
    gameplay_cleanup
};
