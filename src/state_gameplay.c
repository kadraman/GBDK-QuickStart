#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "state_gameplay.h"
#include "../res/background.h"
#include "../res/font.h"
#include "../res/player.h"

/* -----------------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------------- */
#define FONT_FIRST_TILE  BACKGROUND_TILE_COUNT

/* Player physics */
#define GROUND_Y_HW     80U    /* sprite hardware Y when standing on ground    */
#define JUMP_VY        (-6)    /* initial jump velocity (negative = upward)    */
#define ANIM_WALK_SPD   8U     /* vblanks between walk animation frames        */
#define ANIM_IDLE_SPD  20U     /* vblanks between idle animation frames        */
#define WALK_SPEED      1U     /* world pixels per frame                       */
#define MIN_WORLD_X     8U     /* minimum player world-X (hardware X-8)        */

/* Scrolling */
#define SCROLL_R_LIMIT  100U   /* scroll right when screen-X exceeds this      */
#define SCROLL_L_LIMIT   60U   /* scroll left  when screen-X falls below this  */
#define MAX_SCROLL_X    ((BACKGROUND_MAP_WIDTH - 20U) * 8U)  /* 96 px for 32-tile map */
#define MAX_WORLD_X     (BACKGROUND_MAP_WIDTH * 8U - 8U)    /* 248 */

/* HUD window */
#define HUD_WIN_Y       112U   /* window Y: bottom 4 tile rows (32 px)         */
#define HUD_PAL          3U    /* BKG palette slot for HUD text (white)        */
#define HUD_RED_PAL      4U    /* BKG palette slot for hearts (red)            */

/* -----------------------------------------------------------------------
 * Player state
 * -------------------------------------------------------------------- */
typedef enum { PSTATE_IDLE, PSTATE_WALK, PSTATE_JUMP } PlayerState;

static uint8_t      player_world_x;   /* world-space pixel X (0..MAX_WORLD_X)  */
static uint8_t      player_hw_y;      /* hardware sprite Y register             */
static int8_t       player_vy;        /* vertical velocity (negative = up)      */
static uint8_t      player_facing_r;  /* 1 = right, 0 = left                   */
static PlayerState  player_state;
static uint8_t      anim_frame;
static uint8_t      anim_counter;
static uint8_t      camera_x;         /* SCX_REG value (0..MAX_SCROLL_X)        */
static uint16_t     score;
static uint8_t      lives;
static uint8_t      prev_joy;

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
    uint8_t space_tile = (uint8_t)(FONT_FIRST_TILE);   /* space = empty heart slot */
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

    /* Position window at bottom of screen */
    move_win(0, HUD_WIN_Y);

    /* Fill all 4 window rows with HUD background (space tiles + HUD palette) */
    VBK_REG = 0;
    for (row = 0; row < 4U; row++) {
        for (x = 0; x < 20U; x++) {
            set_win_tile_xy(x, row, (uint8_t)(FONT_FIRST_TILE)); /* space tile */
        }
    }
    VBK_REG = 1;
    for (row = 0; row < 4U; row++) {
        for (x = 0; x < 20U; x++) {
            set_win_tile_xy(x, row, HUD_PAL);
        }
    }
    VBK_REG = 0;

    /* Row 1: "SCORE: 0000" */
    hud_write_text(0U, 1U, "SCORE: ", HUD_PAL);
    hud_update_score();

    /* Row 2: "LIVES: " + hearts */
    hud_write_text(0U, 2U, "LIVES: ", HUD_PAL);
    hud_update_lives();
}

/* -----------------------------------------------------------------------
 * State callbacks
 * -------------------------------------------------------------------- */
static void gameplay_init(void)
{
    player_world_x  = 20U;
    player_hw_y     = GROUND_Y_HW;
    player_vy       = 0;
    player_facing_r = 1U;
    player_state    = PSTATE_IDLE;
    anim_frame      = 0;
    anim_counter    = 0;
    camera_x        = 0;
    score           = 0;
    lives           = 3;
    prev_joy        = 0;

    /* Load full background tilemap (32x18) in one call */
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_map);
    /* Load per-tile palette attributes from pre-built attr_map */
    VBK_REG = 1;
    set_bkg_tiles(0, 0, BACKGROUND_MAP_WIDTH, BACKGROUND_MAP_HEIGHT,
                  background_attr_map);
    VBK_REG = 0;

    SCX_REG = 0;
    SCY_REG = 0;

    /* Place player sprite */
    set_sprite_tile(0, PLAYER_ANIM_IDLE_START);
    set_sprite_prop(0, 0);
    move_sprite(0, (uint8_t)(player_world_x - camera_x + 8U), player_hw_y);

    hud_init();
    SHOW_WIN;
}

static void gameplay_update(void)
{
    uint8_t joy        = joypad();
    uint8_t joy_press  = (uint8_t)(joy & ~prev_joy);
    uint8_t moved      = 0;
    int16_t new_y;
    uint8_t screen_x, hw_x;
    uint8_t tile_idx, prop;
    uint8_t anim_speed;
    uint8_t anim_start, anim_frames;

    /* --- Horizontal movement --- */
    if (joy & J_RIGHT) {
        player_facing_r = 1U;
        if (player_world_x < MAX_WORLD_X) {
            player_world_x = (uint8_t)(player_world_x + WALK_SPEED);
            moved = 1U;
        }
    } else if (joy & J_LEFT) {
        player_facing_r = 0U;
        if (player_world_x > MIN_WORLD_X) {
            player_world_x = (uint8_t)(player_world_x - WALK_SPEED);
            moved = 1U;
        }
    }

    /* --- Jump (A or B button, only when grounded) --- */
    if ((joy_press & J_A) || (joy_press & J_B)) {
        if (player_state != PSTATE_JUMP) {
            player_vy    = JUMP_VY;
            player_state = PSTATE_JUMP;
            score++;
            hud_update_score();
        }
    }

    /* --- Vertical physics --- */
    if (player_state == PSTATE_JUMP) {
        new_y = (int16_t)player_hw_y + player_vy;
        if (new_y < 16) new_y = 16;                 /* top of screen clamp  */
        if (new_y >= (int16_t)GROUND_Y_HW) {
            new_y        = (int16_t)GROUND_Y_HW;    /* land on ground       */
            player_vy    = 0;
            player_state = moved ? PSTATE_WALK : PSTATE_IDLE;
        } else {
            player_vy++;                             /* gravity              */
        }
        player_hw_y = (uint8_t)new_y;
    } else {
        /* Ground-state transition */
        PlayerState next = moved ? PSTATE_WALK : PSTATE_IDLE;
        if (next != player_state) {
            player_state = next;
            anim_frame   = 0;
            anim_counter = 0;
        }
    }

    /* --- Camera / scroll --- */
    screen_x = (uint8_t)(player_world_x - camera_x);
    if (screen_x > SCROLL_R_LIMIT && camera_x < MAX_SCROLL_X) {
        camera_x++;
    } else if (screen_x < SCROLL_L_LIMIT && camera_x > 0) {
        camera_x--;
    }
    SCX_REG = camera_x;

    /* --- Animation selection --- */
    if (player_state == PSTATE_JUMP) {
        /* Frame driven by velocity: 0 = rising, 1 = falling */
        anim_frame = (player_vy < 0) ? 0U : 1U;
        tile_idx   = (uint8_t)(PLAYER_ANIM_JUMP_START +
                                anim_frame * PLAYER_TILES_PER_FRAME);
    } else {
        if (player_state == PSTATE_WALK) {
            anim_speed  = ANIM_WALK_SPD;
            anim_start  = PLAYER_ANIM_WALK_START;
            anim_frames = PLAYER_ANIM_WALK_FRAMES;
        } else {
            anim_speed  = ANIM_IDLE_SPD;
            anim_start  = PLAYER_ANIM_IDLE_START;
            anim_frames = PLAYER_ANIM_IDLE_FRAMES;
        }
        anim_counter++;
        if (anim_counter >= anim_speed) {
            anim_counter = 0;
            anim_frame   = (uint8_t)((anim_frame + 1U) % anim_frames);
        }
        tile_idx = (uint8_t)(anim_start + anim_frame * PLAYER_TILES_PER_FRAME);
    }
    set_sprite_tile(0, tile_idx);

    /* --- Horizontal flip for left-facing --- */
    prop = get_sprite_prop(0);
    if (player_facing_r) {
        prop = (uint8_t)(prop & ~S_FLIPX);
    } else {
        prop = (uint8_t)(prop | S_FLIPX);
    }
    set_sprite_prop(0, prop);

    /* --- Move sprite --- */
    hw_x = (uint8_t)(player_world_x - camera_x + 8U);
    move_sprite(0, hw_x, player_hw_y);

    prev_joy    = joy;
}

static void gameplay_cleanup(void)
{
    move_sprite(0, 0, 0);   /* hide sprite */
    HIDE_WIN;               /* hide HUD window */
    SCX_REG = 0;            /* reset background scroll */
}

const GameState state_gameplay = {
    gameplay_init,
    gameplay_update,
    gameplay_cleanup
};
