#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_gameplay.h"
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_player.h"
#include "sprite_enemy.h"
#include "../res/background.h"
#include "../res/font.h"
#include "../res/player.h"
#include "../res/enemy.h"

/* -----------------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------------- */
#define FONT_FIRST_TILE  BACKGROUND_TILE_COUNT

/* World / ground coordinates (world_y = screen top of sprite)
 *   OAM Y = world_y + 16
 *   Player on ground: world_y = 64  -> OAM Y 80, feet at screen Y 79
 *   Enemy  on ground: world_y = 72  -> OAM Y 88, feet at screen Y 79    */
#define GROUND_Y          64U   /* player world_y when standing on ground */
#define ENEMY_GROUND_Y    72U   /* enemy  world_y when standing on ground */

/* Win condition */
#define CHECKPOINT_X     220U   /* world-X that triggers the win state   */

/* HUD window */
#define HUD_WIN_Y        112U   /* window Y: bottom 4 tile rows (32 px)  */
#define HUD_PAL            3U   /* BKG palette slot for HUD text (white) */
#define HUD_RED_PAL        4U   /* BKG palette slot for hearts (red)     */

/* Collision */
#define COLLISION_COOLDOWN  60U  /* vblanks of invincibility after a hit */

/* -----------------------------------------------------------------------
 * Game state
 * -------------------------------------------------------------------- */
static uint8_t  camera_x;
static uint16_t score;
static uint8_t  lives;
static uint8_t  prev_joy;
static uint8_t  collision_cooldown;

/* -----------------------------------------------------------------------
 * HUD helpers
 * -------------------------------------------------------------------- */
static void hud_write_text(uint8_t x, uint8_t y, const char *str, uint8_t pal)
{
    uint8_t i = 0;
    VBK_REG = 0;
    while (str[i]) {
        set_win_tile_xy((uint8_t)(x + i), y,
                        (uint8_t)(FONT_FIRST_TILE + str[i] - 32U));
        i++;
    }
    VBK_REG = 1;
    for (uint8_t j = 0; j < i; j++) {
        set_win_tile_xy((uint8_t)(x + j), y, pal);
    }
    VBK_REG = 0;
}

static void hud_update_score(void)
{
    uint16_t s = score;
    uint8_t d3 = (uint8_t)(s % 10U); s /= 10U;
    uint8_t d2 = (uint8_t)(s % 10U); s /= 10U;
    uint8_t d1 = (uint8_t)(s % 10U); s /= 10U;
    uint8_t d0 = (uint8_t)(s % 10U);

    VBK_REG = 0;
    set_win_tile_xy(7U, 1U, (uint8_t)(FONT_FIRST_TILE + d0 + ('0' - 32U)));
    set_win_tile_xy(8U, 1U, (uint8_t)(FONT_FIRST_TILE + d1 + ('0' - 32U)));
    set_win_tile_xy(9U, 1U, (uint8_t)(FONT_FIRST_TILE + d2 + ('0' - 32U)));
    set_win_tile_xy(10U, 1U, (uint8_t)(FONT_FIRST_TILE + d3 + ('0' - 32U)));
    VBK_REG = 1;
    set_win_tile_xy(7U, 1U, HUD_PAL);
    set_win_tile_xy(8U, 1U, HUD_PAL);
    set_win_tile_xy(9U, 1U, HUD_PAL);
    set_win_tile_xy(10U, 1U, HUD_PAL);
    VBK_REG = 0;
}

static void hud_update_lives(void)
{
    uint8_t heart_tile = (uint8_t)(FONT_FIRST_TILE + FONT_TILE_HEART);
    uint8_t space_tile = (uint8_t)(FONT_FIRST_TILE);
    uint8_t i;

    VBK_REG = 0;
    for (i = 0; i < 3U; i++) {
        set_win_tile_xy((uint8_t)(7U + i), 2U,
                        (i < lives) ? heart_tile : space_tile);
    }
    VBK_REG = 1;
    for (i = 0; i < 3U; i++) {
        set_win_tile_xy((uint8_t)(7U + i), 2U, HUD_RED_PAL);
    }
    VBK_REG = 0;
}

static void hud_init(void)
{
    uint8_t x, row;

    move_win(7U, HUD_WIN_Y);

    VBK_REG = 0;
    for (row = 0; row < 4U; row++) {
        for (x = 0; x < 20U; x++) {
            set_win_tile_xy(x, row, (uint8_t)(FONT_FIRST_TILE));
        }
    }
    VBK_REG = 1;
    for (row = 0; row < 4U; row++) {
        for (x = 0; x < 20U; x++) {
            set_win_tile_xy(x, row, HUD_PAL);
        }
    }
    VBK_REG = 0;

    hud_write_text(0U, 1U, "SCORE: ", HUD_PAL);
    hud_update_score();
    hud_write_text(0U, 2U, "LIVES: ", HUD_PAL);
    hud_update_lives();
}

/* -----------------------------------------------------------------------
 * State callbacks
 * -------------------------------------------------------------------- */
static void gameplay_init(void)
{
    camera_x           = 0;
    score              = 0;
    lives              = 3;
    prev_joy           = 0;
    collision_cooldown = 0;

    sprite_manager_init();

    /* Player: 16x16 -> 2 OBJ slots */
    player_init(20U, GROUND_Y, 0U);

    /* Enemy: 8x8 -> 1 OBJ slot; tile_base after player tiles */
    enemy_init(80U, ENEMY_GROUND_Y, PLAYER_TILE_COUNT);

    /* Load background tilemap */
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_map);
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_attr_map);
    VBK_REG = 0;

    SCX_REG = 0;
    SCY_REG = 0;

    hud_init();
    SHOW_WIN;
}

static void gameplay_update(void)
{
    uint8_t joy       = joypad();
    uint8_t joy_press = (uint8_t)(joy & ~prev_joy);
    uint8_t events;

    /* --- Update player (handles movement, physics, animation, camera) --- */
    events = player_update(joy, joy_press, &camera_x);

    if (events & PLAYER_EVENT_JUMPED) {
        score++;
        hud_update_score();
    }

    /* --- Fell into a gap: lose a life and restart or game over --- */
    if (events & PLAYER_EVENT_FELL_GAP) {
        if (lives > 0U) {
            lives--;
            hud_update_lives();
        }
        if (lives == 0U) {
            switch_state(STATE_GAME_OVER);
        } else {
            switch_state(STATE_GAMEPLAY);  /* restart from beginning */
        }
        return;
    }

    /* --- Update enemy (handles patrol, animation, hardware move) --- */
    enemy_update(camera_x);

    /* --- Win condition: player reaches checkpoint --- */
    if (player_get_sprite()->world_x >= CHECKPOINT_X) {
        switch_state(STATE_WIN);
        return;
    }

    /* --- Sprite collision: player vs enemy --- */
    if (collision_cooldown > 0U) {
        collision_cooldown--;
    } else if (sprites_collide(player_get_sprite(), enemy_get_sprite())) {
        if (lives > 0U) {
            lives--;
            hud_update_lives();
        }
        if (lives == 0U) {
            switch_state(STATE_GAME_OVER);
            return;
        }
        collision_cooldown = COLLISION_COOLDOWN;
    }

    prev_joy = joy;
}

static void gameplay_cleanup(void)
{
    player_cleanup();
    enemy_cleanup();
    HIDE_WIN;
    SCX_REG = 0;
}

const GameState state_gameplay = {
    gameplay_init,
    gameplay_update,
    gameplay_cleanup
};
