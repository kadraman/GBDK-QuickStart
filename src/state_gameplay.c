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
/* Font starts at VRAM tile 128 (fixed across all states) */
#define FONT_FIRST_TILE  128U

/* Player physics */
#define GROUND_Y_HW      80U
#define PLATFORM_Y_HW    56U
#define PIT_DEATH_Y_HW  176U
#define JUMP_VY         (-6)
#define ANIM_WALK_SPD    8U
#define ANIM_IDLE_SPD   20U
#define WALK_SPEED       1U
#define MIN_WORLD_X      8U

/* Scrolling */
#define SCROLL_R_LIMIT  100U
#define SCROLL_L_LIMIT   60U
#define MAX_SCROLL_X    224U   /* (48-20)*8 = 224 */
#define MAX_WORLD_X     376U   /* 48*8-8 = 376    */

/* HUD */
#define HUD_WIN_Y       112U
#define HUD_PAL          3U
#define HUD_RED_PAL      4U

/* Timer: 60 seconds at ~60 vblanks/sec */
#define TIMER_START     3600U

/* -----------------------------------------------------------------------
 * Level collision zones
 *
 * Pit zones (world-x ranges with no ground floor):
 *   PIT1: cols 10-12  => world-x  80..103
 *   PIT2: cols 21-24  => world-x 168..199
 *   PIT3: cols 34-39  => world-x 272..319
 *
 * Platform zones (raised platforms at PLATFORM_Y_HW):
 *   PLAT1: col 7      => world-x  56..63
 *   PLAT2: cols 15-16 => world-x 120..135
 *   PLAT3: cols 27-28 => world-x 216..231
 *   PLAT4: col 42     => world-x 336..343
 * -------------------------------------------------------------------- */

static uint8_t is_over_pit(uint16_t wx)
{
    if (wx >= 80U  && wx <= 103U) return 1U;
    if (wx >= 168U && wx <= 199U) return 1U;
    if (wx >= 272U && wx <= 319U) return 1U;
    return 0U;
}

static uint8_t get_platform_y(uint16_t wx)
{
    if (wx >= 56U  && wx <= 63U)  return PLATFORM_Y_HW;
    if (wx >= 120U && wx <= 135U) return PLATFORM_Y_HW;
    if (wx >= 216U && wx <= 231U) return PLATFORM_Y_HW;
    if (wx >= 336U && wx <= 343U) return PLATFORM_Y_HW;
    return 255U;
}

/* -----------------------------------------------------------------------
 * Column streaming
 *
 * The GBC hardware background is a 32x32 tile ring buffer.  We store
 * a 48x18 level map in ROM and stream one column at a time into the
 * ring buffer as the camera scrolls right.
 *
 * Ring-buffer safety: once bg_stream_right > 32, ring positions
 * 0..(bg_stream_right-33) have been overwritten by columns 32+.
 * The camera left tile must stay >= (bg_stream_right - 32) to avoid
 * showing stale data.  This is enforced via min_world_x.
 * -------------------------------------------------------------------- */
static uint8_t bg_stream_right;

static void load_bg_column(uint8_t level_col)
{
    uint8_t bg_col = (uint8_t)(level_col % 32U);
    uint8_t row;
    VBK_REG = 0;
    for (row = 0; row < BACKGROUND_MAP_HEIGHT; row++) {
        set_bkg_tile_xy(bg_col, row,
            background_map[(uint16_t)row * BACKGROUND_MAP_WIDTH + level_col]);
    }
    VBK_REG = 1;
    for (row = 0; row < BACKGROUND_MAP_HEIGHT; row++) {
        set_bkg_tile_xy(bg_col, row,
            background_attr_map[(uint16_t)row * BACKGROUND_MAP_WIDTH + level_col]);
    }
    VBK_REG = 0;
}

/* -----------------------------------------------------------------------
 * Player state
 * -------------------------------------------------------------------- */
typedef enum { PSTATE_IDLE, PSTATE_WALK, PSTATE_JUMP } PlayerState;

static uint16_t     player_world_x;
static uint8_t      player_hw_y;
static int8_t       player_vy;
static uint8_t      player_facing_r;
static PlayerState  player_state;
static uint8_t      on_platform;
static uint8_t      anim_frame;
static uint8_t      anim_counter;
static uint8_t      camera_x;
static uint16_t     min_world_x;
static uint16_t     score;
static uint8_t      lives;
static uint16_t     time_remaining;
static uint8_t      last_seconds;
static uint8_t      prev_joy;

static const palette_color_t gameplay_font_palette[4] = {
    RGB8(155, 200, 234),
    RGB8(  0,   0,   0),
    RGB8(170, 170, 170),
    RGB8( 85,  85,  85),
};

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
 * Respawn player at safe position after falling into a pit
 * -------------------------------------------------------------------- */
static void respawn_player(void)
{
    player_world_x = (uint16_t)(min_world_x + MIN_WORLD_X);
    player_hw_y    = GROUND_Y_HW;
    player_vy      = 0;
    player_state   = PSTATE_IDLE;
    on_platform    = 0;
    anim_frame     = 0;
    anim_counter   = 0;
    if (player_world_x >= (uint16_t)SCROLL_L_LIMIT) {
        camera_x = (uint8_t)(player_world_x - (uint16_t)SCROLL_L_LIMIT);
    } else {
        camera_x = 0U;
    }
    if (camera_x > MAX_SCROLL_X) camera_x = MAX_SCROLL_X;
    SCX_REG = camera_x;
    move_sprite(0, (uint8_t)(player_world_x - (uint16_t)camera_x + 8U),
                player_hw_y);
}

/* -----------------------------------------------------------------------
 * State callbacks
 * -------------------------------------------------------------------- */
static void gameplay_init(void)
{
    uint8_t col;

    player_world_x  = (uint16_t)(MIN_WORLD_X + 12U);
    player_hw_y     = GROUND_Y_HW;
    player_vy       = 0;
    player_facing_r = 1U;
    player_state    = PSTATE_IDLE;
    on_platform     = 0;
    anim_frame      = 0;
    anim_counter    = 0;
    camera_x        = 0;
    min_world_x     = (uint16_t)MIN_WORLD_X;
    score           = 0;
    lives           = 3;
    time_remaining  = TIMER_START;
    last_seconds    = 60U;
    prev_joy        = 0;
    bg_stream_right = 0;

    set_bkg_data(0, BACKGROUND_TILE_COUNT, background_tiles);
    set_bkg_palette(0, BACKGROUND_PALETTE_COUNT, background_palettes);
    set_bkg_palette(2, 1, gameplay_font_palette);

    for (col = 0; col < 32U; col++) {
        load_bg_column(col);
    }
    bg_stream_right = 32U;

    SCX_REG = 0;
    SCY_REG = 0;

    set_sprite_tile(0, PLAYER_ANIM_IDLE_START);
    set_sprite_prop(0, 0);
    move_sprite(0, (uint8_t)(player_world_x + 8U), player_hw_y);

    hud_init();
    SHOW_WIN;
}

static void gameplay_update(void)
{
    uint8_t  joy        = joypad();
    uint8_t  joy_press  = (uint8_t)(joy & ~prev_joy);
    uint8_t  moved      = 0;
    int16_t  new_y;
    uint8_t  screen_x, hw_x;
    uint8_t  tile_idx, prop;
    uint8_t  anim_speed;
    uint8_t  anim_start, anim_frames;
    uint8_t  floor_y, plat_y;
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

    /* --- Ring-buffer safety: advance min_world_x as we stream right ---
     * Camera left tile must stay >= (bg_stream_right - 32) at all times.
     * Player must stay >= (bg_stream_right-32)*8 + SCROLL_L_LIMIT so the
     * camera never exposes overwritten ring-buffer columns.              */
    if (bg_stream_right > 32U) {
        uint16_t safe_x = (uint16_t)((uint16_t)(bg_stream_right - 32U) * 8U
                                     + (uint16_t)SCROLL_L_LIMIT);
        if (safe_x > min_world_x) min_world_x = safe_x;
    }

    /* --- Horizontal movement --- */
    if (joy & J_RIGHT) {
        player_facing_r = 1U;
        if (player_world_x < (uint16_t)MAX_WORLD_X) {
            player_world_x = (uint16_t)(player_world_x + WALK_SPEED);
            moved = 1U;
        }
    } else if (joy & J_LEFT) {
        player_facing_r = 0U;
        if (player_world_x > min_world_x) {
            player_world_x = (uint16_t)(player_world_x - WALK_SPEED);
            moved = 1U;
        }
    }

    /* --- Jump (A or B, only when grounded or on platform) --- */
    if ((joy_press & J_A) || (joy_press & J_B)) {
        if (player_state != PSTATE_JUMP) {
            player_vy    = JUMP_VY;
            player_state = PSTATE_JUMP;
            on_platform  = 0;
            score++;
            hud_update_score();
        }
    }

    /* --- Determine floor/platform at current world position --- */
    plat_y  = get_platform_y(player_world_x);
    floor_y = is_over_pit(player_world_x) ? 255U : GROUND_Y_HW;

    /* --- Grounded but walked into pit: start falling --- */
    if (player_state != PSTATE_JUMP && !on_platform && floor_y == 255U) {
        player_state = PSTATE_JUMP;
        player_vy    = 0;
    }

    /* --- On platform but walked off edge: start falling --- */
    if (on_platform && plat_y == 255U) {
        on_platform  = 0;
        player_state = PSTATE_JUMP;
        player_vy    = 0;
    }

    /* --- Vertical physics --- */
    if (player_state == PSTATE_JUMP) {
        new_y = (int16_t)player_hw_y + (int16_t)player_vy;
        if (new_y < 16) new_y = 16;

        /* Platform landing (only when falling downward) */
        if (player_vy > 0 && plat_y != 255U) {
            if ((int16_t)player_hw_y <= (int16_t)plat_y &&
                new_y >= (int16_t)plat_y) {
                new_y        = (int16_t)plat_y;
                player_vy    = 0;
                player_state = moved ? PSTATE_WALK : PSTATE_IDLE;
                on_platform  = 1;
            }
        }

        /* Ground landing */
        if (player_state == PSTATE_JUMP && player_vy >= 0 && floor_y != 255U) {
            if (new_y >= (int16_t)floor_y) {
                new_y        = (int16_t)floor_y;
                player_vy    = 0;
                player_state = moved ? PSTATE_WALK : PSTATE_IDLE;
                on_platform  = 0;
            }
        }

        if (player_state == PSTATE_JUMP) {
            player_vy++;
        }

        player_hw_y = (uint8_t)new_y;

        /* Fell below screen: lose a life and respawn */
        if (player_hw_y >= PIT_DEATH_Y_HW) {
            if (lives > 0U) lives--;
            hud_update_lives();
            if (lives == 0U) {
                switch_state(STATE_GAME_OVER);
                return;
            }
            respawn_player();
            prev_joy = joy;
            return;
        }
    } else {
        player_hw_y = on_platform ? PLATFORM_Y_HW : GROUND_Y_HW;

        {
            PlayerState next = moved ? PSTATE_WALK : PSTATE_IDLE;
            if (next != player_state) {
                player_state = next;
                anim_frame   = 0;
                anim_counter = 0;
            }
        }
    }

    /* --- Camera / scroll --- */
    screen_x = (uint8_t)(player_world_x - (uint16_t)camera_x);
    if (screen_x > SCROLL_R_LIMIT && camera_x < MAX_SCROLL_X) {
        camera_x++;
    } else if (screen_x < SCROLL_L_LIMIT && camera_x > 0U) {
        camera_x--;
    }
    SCX_REG = camera_x;

    /* --- Column streaming (rightward only) --- */
    cam_tile   = (uint8_t)(camera_x >> 3);
    needed_col = (uint8_t)(cam_tile + 21U);
    if (needed_col < BACKGROUND_MAP_WIDTH && needed_col >= bg_stream_right) {
        load_bg_column(bg_stream_right);
        bg_stream_right++;
    }

    /* --- Animation --- */
    if (player_state == PSTATE_JUMP) {
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

    prop = get_sprite_prop(0);
    if (player_facing_r) {
        prop = (uint8_t)(prop & ~S_FLIPX);
    } else {
        prop = (uint8_t)(prop | S_FLIPX);
    }
    set_sprite_prop(0, prop);

    hw_x = (uint8_t)(player_world_x - (uint16_t)camera_x + 8U);
    move_sprite(0, hw_x, player_hw_y);

    prev_joy = joy;
}

static void gameplay_cleanup(void)
{
    move_sprite(0, 0, 0);
    HIDE_WIN;
    SCX_REG = 0;
}

const GameState state_gameplay = {
    gameplay_init,
    gameplay_update,
    gameplay_cleanup
};
