#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_gameplay.h"
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_player.h"
#include "sprite_enemy.h"
#include "bg_gameplay.h"
#include "font.h"
#include "player.h"
#include "enemy.h"

/* -----------------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------------- */
/* Font starts immediately after background tiles in VRAM */
#define FONT_FIRST_TILE  BG_GAMEPLAY_TILE_COUNT

/* Win condition: reached the end of the 48-tile level */
#define CHECKPOINT_X16   360U   /* world-X >= 360 triggers win           */

/* HUD window */
#define HUD_WIN_Y        112U   /* window Y: bottom 4 tile rows (32 px)  */
#define HUD_PAL            3U   /* BKG palette slot for HUD text (white) */
#define HUD_RED_PAL        4U   /* BKG palette slot for hearts (red)     */

/* Collision */
#define COLLISION_COOLDOWN  60U  /* vblanks of invincibility after a hit */

/* Timer: 60 seconds at ~60 vblanks/sec */
#define TIMER_START      3600U

/* Column streaming constants (48-tile level, 32-tile ring buffer) */
#define SCROLL_L_LIMIT     60U   /* must match sprite_player.c           */

/* -----------------------------------------------------------------------
 * Font palette for gameplay sky
 * -------------------------------------------------------------------- */
static const palette_color_t gameplay_font_palette[4] = {
    RGB8(155, 200, 234),   /* 0 - sky blue background */
    RGB8(  0,   0,   0),   /* 1 - black text          */
    RGB8(170, 170, 170),   /* 2 - unused              */
    RGB8( 85,  85,  85),   /* 3 - unused              */
};

/* -----------------------------------------------------------------------
 * Game state
 * -------------------------------------------------------------------- */
static uint8_t  camera_x;
static uint16_t score;
static uint8_t  lives;
static uint8_t  prev_joy;
static uint8_t  collision_cooldown;
static uint16_t time_remaining;
static uint8_t  last_seconds;
static uint8_t  bg_stream_right;   /* next column to stream into the ring buffer */

/* -----------------------------------------------------------------------
 * Column streaming
 *
 * The GBC hardware background is a 32x32 tile ring buffer.  We store
 * a 48x18 level map in ROM and stream one column at a time into the
 * ring buffer as the camera scrolls right.
 *
 * Banking note: bg_gameplay_map and bg_gameplay_attr_map live in ROM
 * bank 1 (asset bank).  SWITCH_ROM is called here so the function
 * remains correct if auto-banking eventually places these arrays in a
 * bank other than the one containing this code.
 * -------------------------------------------------------------------- */
static void load_bg_column(uint8_t level_col)
{
    uint8_t bg_col = (uint8_t)(level_col % 32U);
    uint8_t row;

    SWITCH_ROM(BANK(bg_gameplay_tiles));

    VBK_REG = 0;
    for (row = 0; row < BG_GAMEPLAY_MAP_HEIGHT; row++) {
        set_bkg_tile_xy(bg_col, row,
            bg_gameplay_map[(uint16_t)row * BG_GAMEPLAY_MAP_WIDTH + level_col]);
    }
    VBK_REG = 1;
    for (row = 0; row < BG_GAMEPLAY_MAP_HEIGHT; row++) {
        set_bkg_tile_xy(bg_col, row,
            bg_gameplay_attr_map[(uint16_t)row * BG_GAMEPLAY_MAP_WIDTH + level_col]);
    }
    VBK_REG = 0;

    SWITCH_ROM(1);
}

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

static void hud_update_time(void)
{
    uint8_t secs = last_seconds;
    uint8_t d1   = (uint8_t)(secs / 10U);
    uint8_t d0   = (uint8_t)(secs % 10U);
    uint8_t i;

    VBK_REG = 0;
    set_win_tile_xy(12U, 1U, (uint8_t)(FONT_FIRST_TILE + 'T' - 32U));
    set_win_tile_xy(13U, 1U, (uint8_t)(FONT_FIRST_TILE + 'I' - 32U));
    set_win_tile_xy(14U, 1U, (uint8_t)(FONT_FIRST_TILE + 'M' - 32U));
    set_win_tile_xy(15U, 1U, (uint8_t)(FONT_FIRST_TILE + 'E' - 32U));
    set_win_tile_xy(16U, 1U, (uint8_t)(FONT_FIRST_TILE + ':' - 32U));
    set_win_tile_xy(17U, 1U, (uint8_t)(FONT_FIRST_TILE + ' ' - 32U));
    set_win_tile_xy(18U, 1U, (uint8_t)(FONT_FIRST_TILE + d1 + ('0' - 32U)));
    set_win_tile_xy(19U, 1U, (uint8_t)(FONT_FIRST_TILE + d0 + ('0' - 32U)));
    VBK_REG = 1;
    for (i = 12U; i < 20U; i++) {
        set_win_tile_xy(i, 1U, HUD_PAL);
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
    hud_update_time();
    hud_write_text(0U, 2U, "LIVES: ", HUD_PAL);
    hud_update_lives();
}

/* -----------------------------------------------------------------------
 * State callbacks
 * -------------------------------------------------------------------- */
static void gameplay_init(void)
{
    uint8_t col;

    camera_x           = 0;
    score              = 0;
    lives              = 3;
    prev_joy           = 0;
    collision_cooldown = 0;
    time_remaining     = TIMER_START;
    last_seconds       = 60U;
    bg_stream_right    = 0;

    sprite_manager_init();

    /* Switch to asset bank before loading ROM data into VRAM/palettes.
     * load_bg_column() also calls SWITCH_ROM internally. */
    SWITCH_ROM(BANK(bg_gameplay_tiles));

    /* Load gameplay background tiles (slot 0..BG_GAMEPLAY_TILE_COUNT-1) */
    set_bkg_data(0, BG_GAMEPLAY_TILE_COUNT, bg_gameplay_tiles);
    /* Font tiles immediately after background tiles */
    set_bkg_data(BG_GAMEPLAY_TILE_COUNT, FONT_TILE_COUNT, font_tiles);

    /* Background palettes (slots 0-1: sky and ground) */
    set_bkg_palette(0, BG_GAMEPLAY_PALETTE_COUNT, bg_gameplay_palettes);
    /* Font palette: sky-blue background, black text (slot 2) */
    set_bkg_palette(2, 1, gameplay_font_palette);

    /* Restore game code bank */
    SWITCH_ROM(1);

    /* Player: 16x16 -> 2 OBJ slots */
    player_init(20U, 64U, 0U);

    /* Enemy: 8x8 -> 1 OBJ slot; tile_base after player tiles */
    enemy_init(80U, 72U, PLAYER_TILE_COUNT);

    /* Load initial 32 columns into hardware background ring buffer.
     * load_bg_column() handles its own bank switch internally. */
    for (col = 0; col < 32U; col++) {
        load_bg_column(col);
    }
    bg_stream_right = 32U;

    SCX_REG = 0;
    SCY_REG = 0;

    hud_init();
    SHOW_WIN;
}

static void gameplay_update(void)
{
    uint8_t  joy        = joypad();
    uint8_t  joy_press  = (uint8_t)(joy & ~prev_joy);
    uint8_t  events;
    uint16_t min_world_x;
    uint8_t  cam_tile, needed_col;

    /* --- Countdown timer --- */
    if (time_remaining > 0U) {
        time_remaining--;
        {
            uint8_t secs = (uint8_t)(time_remaining / 60U);
            if (secs != last_seconds) {
                last_seconds = secs;
                hud_update_time();
            }
        }
    } else {
        switch_state(STATE_GAME_OVER);
        return;
    }

    /* --- Ring-buffer safety: compute min_world_x ---
     * Camera left tile must stay >= (bg_stream_right - 32) at all times.
     * Player must stay >= (bg_stream_right-32)*8 + SCROLL_L_LIMIT so the
     * camera never exposes overwritten ring-buffer columns.              */
    if (bg_stream_right > 32U) {
        min_world_x = (uint16_t)((uint16_t)(bg_stream_right - 32U) * 8U
                                 + (uint16_t)SCROLL_L_LIMIT);
    } else {
        min_world_x = 8U;  /* MIN_WORLD_X from sprite_player.c */
    }

    /* --- Update player (handles movement, physics, animation, camera) --- */
    events = player_update(joy, joy_press, &camera_x, min_world_x);

    if (events & PLAYER_EVENT_JUMPED) {
        score++;
        hud_update_score();
    }

    /* --- Fell into a pit: lose a life and restart or game over --- */
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

    /* --- Win condition: player reaches end of level --- */
    if (player_get_world_x16() >= (uint16_t)CHECKPOINT_X16) {
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

    /* --- Column streaming (rightward only) ---
     * Pre-load columns just off the right edge of the screen.           */
    cam_tile   = (uint8_t)(camera_x >> 3);
    needed_col = (uint8_t)(cam_tile + 21U);
    if (needed_col < BG_GAMEPLAY_MAP_WIDTH && needed_col >= bg_stream_right) {
        load_bg_column(bg_stream_right);
        bg_stream_right++;
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
