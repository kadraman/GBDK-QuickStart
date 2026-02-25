#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_player.h"
#include "../res/player.h"

/* -----------------------------------------------------------------------
 * Player physics constants
 * -------------------------------------------------------------------- */
#define JUMP_VY          (-5)   /* initial jump velocity (negative = up) */
#define WALK_SPEED         1U   /* world pixels per frame                */
#define GRAVITY_DELAY      2U   /* frames between gravity steps (1 = every frame) */
#define MIN_WORLD_X        8U   /* leftmost player world-X               */
#define MAX_WORLD_X      248U   /* rightmost player world-X              */
#define SCROLL_R_LIMIT   100U   /* scroll right when screen-X exceeds    */
#define SCROLL_L_LIMIT    60U   /* scroll left  when screen-X falls below */
#define MAX_SCROLL_X      96U   /* maximum SCX value (32-tile map)       */

/* Maximum world_y before the player is considered to have fallen off */
#define MAX_FALL_WORLD_Y  160U

/* Wall table: {left_x, right_x} – world X ranges that block movement.
 * Must match the _WALLS set in tools/gen_background.py.
 * col 13 → world_x 104-111,  col 24 → world_x 192-199               */
static const uint8_t _walls[][2] = {
    {104U, 112U},
    {192U, 200U},
};
#define NUM_WALLS  2U

/* Gap table: {start_x, end_x} – world X ranges with no ground.
 * Must match the _GAPS set in tools/gen_background.py.
 * cols 9-10 → 72-87,  cols 19-20 → 152-167                           */
static const uint8_t _gaps[][2] = {
    { 72U,  88U},
    {152U, 168U},
};
#define NUM_GAPS  2U

/* Wall clear height: player world_y must be at or above this to pass over
 * a wall tile (row 9 = screen Y 72, so world_y <= 56 clears it).       */
#define WALL_CLEAR_WORLD_Y  56U

typedef enum { PSTATE_IDLE, PSTATE_WALK, PSTATE_JUMP } PlayerState;

static Sprite      *_player_sprite;
static int8_t       _player_vy;
static uint8_t      _player_facing_r;
static PlayerState  _player_state;
static uint8_t      _ground_y;
static uint8_t      _gravity_delay_ctr;

/* Returns 1 if the player's horizontal extent overlaps a gap */
static uint8_t _over_gap(void)
{
    uint8_t i;
    uint8_t px_l = _player_sprite->world_x;
    uint8_t px_r = (uint8_t)(px_l + 7U);
    for (i = 0U; i < NUM_GAPS; i++) {
        if (px_r >= _gaps[i][0] && px_l < _gaps[i][1]) {
            return 1U;
        }
    }
    return 0U;
}

/* Returns 1 if moving to new_x would put the player inside a wall while
 * at ground level (not cleared).  dx > 0 = moving right, dx < 0 = left. */
static uint8_t _hits_wall(uint8_t new_x, int8_t dx)
{
    uint8_t i;
    uint8_t px_l, px_r;
    if (_player_sprite->world_y > WALL_CLEAR_WORLD_Y) {
        /* Player is not high enough to clear the wall */
        px_l = new_x;
        px_r = (uint8_t)(new_x + 7U);
        for (i = 0U; i < NUM_WALLS; i++) {
            if (dx > 0) {
                /* Moving right: check right edge vs wall left edge */
                if (px_r >= _walls[i][0] && px_l < _walls[i][1]) return 1U;
            } else {
                /* Moving left: check left edge vs wall right edge */
                if (px_l < _walls[i][1] && px_r >= _walls[i][0]) return 1U;
            }
        }
    }
    return 0U;
}

void player_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base)
{
    _player_vy       = 0;
    _player_facing_r = 1U;
    _player_state    = PSTATE_IDLE;
    _ground_y        = ground_y;
    _gravity_delay_ctr = 0U;

    _player_sprite = sprite_manager_alloc(
        0U, 2U, 8U, 16U, tile_base, PLAYER_TILES_PER_FRAME);
    _player_sprite->world_x    = start_x;
    _player_sprite->world_y    = ground_y;
    _player_sprite->anim_speed = PLAYER_ANIM_IDLE_SPEED;

    /* Initialise OBJ tiles (idle frame 0) */
    set_sprite_tile(0U, PLAYER_ANIM_IDLE_START);
    set_sprite_tile(1U, (uint8_t)(PLAYER_ANIM_IDLE_START + 2U));
    set_sprite_prop(0U, 0U);
    set_sprite_prop(1U, 0U);
    sprite_manager_update_hw(_player_sprite, 0U, 0U);
}

uint8_t player_update(uint8_t joy, uint8_t joy_press, uint8_t *camera_x)
{
    uint8_t events    = 0U;
    uint8_t moved     = 0U;
    uint8_t new_x;
    int16_t new_y;
    uint8_t screen_x;
    uint8_t hw_x;
    uint8_t tile_idx;
    uint8_t prop;
    uint8_t anim_start, anim_frames;

    /* --- Horizontal movement --- */
    if (joy & J_RIGHT) {
        _player_facing_r = 1U;
        if (_player_sprite->world_x < MAX_WORLD_X) {
            new_x = (uint8_t)(_player_sprite->world_x + WALK_SPEED);
            if (!_hits_wall(new_x, 1)) {
                _player_sprite->world_x = new_x;
                moved = 1U;
            }
        }
    } else if (joy & J_LEFT) {
        _player_facing_r = 0U;
        if (_player_sprite->world_x > MIN_WORLD_X) {
            new_x = (uint8_t)(_player_sprite->world_x - WALK_SPEED);
            if (!_hits_wall(new_x, -1)) {
                _player_sprite->world_x = new_x;
                moved = 1U;
            }
        }
    }

    /* --- Jump (A or B button, only when grounded) --- */
    if ((joy_press & J_A) || (joy_press & J_B)) {
        if (_player_state != PSTATE_JUMP) {
            _player_vy    = JUMP_VY;
            _gravity_delay_ctr = 0U;
            _player_state = PSTATE_JUMP;
            events |= PLAYER_EVENT_JUMPED;
        }
    }

    /* --- Vertical physics --- */
    if (_player_state == PSTATE_JUMP || _over_gap()) {
        new_y = (int16_t)_player_sprite->world_y + _player_vy;
        if (new_y < 0) new_y = 0;
        if (new_y >= (int16_t)_ground_y && !_over_gap()) {
            /* Landed on solid ground */
            new_y         = (int16_t)_ground_y;
            _player_vy    = 0;
            _gravity_delay_ctr = 0U;
            _player_state = moved ? PSTATE_WALK : PSTATE_IDLE;
        } else {
            /* gravity applied every GRAVITY_DELAY frames to slow fall */
            _gravity_delay_ctr++;
            if (_gravity_delay_ctr >= GRAVITY_DELAY) {
                _gravity_delay_ctr = 0U;
                _player_vy++;  /* gravity */
            }
        }
        _player_sprite->world_y = (uint8_t)new_y;

        /* Fell off the bottom */
        if (_player_sprite->world_y >= MAX_FALL_WORLD_Y) {
            events |= PLAYER_EVENT_FELL_GAP;
        }
    } else {
        PlayerState next = moved ? PSTATE_WALK : PSTATE_IDLE;
        if (next != _player_state) {
            _player_state                  = next;
            _player_sprite->anim_frame     = 0;
            _player_sprite->anim_counter   = 0;
            _player_sprite->anim_speed     = (next == PSTATE_WALK)
                                             ? PLAYER_ANIM_WALK_SPEED
                                             : PLAYER_ANIM_IDLE_SPEED;
        }
    }

    /* --- Camera / scroll --- */
    screen_x = (uint8_t)(_player_sprite->world_x - *camera_x);
    if (screen_x > SCROLL_R_LIMIT && *camera_x < MAX_SCROLL_X) {
        (*camera_x)++;
    } else if (screen_x < SCROLL_L_LIMIT && *camera_x > 0U) {
        (*camera_x)--;
    }
    SCX_REG = *camera_x;

    /* --- Animation selection --- */
    if (_player_state == PSTATE_JUMP || _over_gap()) {
        _player_sprite->anim_frame = (_player_vy < 0) ? 0U : 1U;
        tile_idx = (uint8_t)(PLAYER_ANIM_JUMP_START +
                              _player_sprite->anim_frame * PLAYER_TILES_PER_FRAME);
    } else {
        anim_start  = (_player_state == PSTATE_WALK) ? PLAYER_ANIM_WALK_START
                                                      : PLAYER_ANIM_IDLE_START;
        anim_frames = (_player_state == PSTATE_WALK) ? PLAYER_ANIM_WALK_FRAMES
                                                      : PLAYER_ANIM_IDLE_FRAMES;
        _player_sprite->anim_counter++;
        if (_player_sprite->anim_counter >= _player_sprite->anim_speed) {
            _player_sprite->anim_counter = 0U;
            _player_sprite->anim_frame   =
                (uint8_t)((_player_sprite->anim_frame + 1U) % anim_frames);
        }
        tile_idx = (uint8_t)(anim_start +
                              _player_sprite->anim_frame * PLAYER_TILES_PER_FRAME);
    }

    /* 16x16 player: OBJ 0 = left half, OBJ 1 = right half */
    set_sprite_tile(0U, tile_idx);
    set_sprite_tile(1U, (uint8_t)(tile_idx + 2U));

    /* --- Horizontal flip for left-facing --- */
    prop = get_sprite_prop(0U);
    if (_player_facing_r) {
        prop = (uint8_t)(prop & ~S_FLIPX);
    } else {
        prop = (uint8_t)(prop | S_FLIPX);
    }
    set_sprite_prop(0U, prop);
    prop = get_sprite_prop(1U);
    if (_player_facing_r) {
        prop = (uint8_t)(prop & ~S_FLIPX);
    } else {
        prop = (uint8_t)(prop | S_FLIPX);
    }
    set_sprite_prop(1U, prop);

    /* --- Move player OBJ slots --- */
    hw_x = (uint8_t)(_player_sprite->world_x - *camera_x + 8U);
    move_sprite(0U, hw_x, (uint8_t)(_player_sprite->world_y + 16U));
    move_sprite(1U, (uint8_t)(hw_x + 8U), (uint8_t)(_player_sprite->world_y + 16U));

    return events;
}

void player_cleanup(void)
{
    if (_player_sprite) {
        sprite_manager_free(_player_sprite);
        _player_sprite = 0;
    }
}

Sprite* player_get_sprite(void)
{
    return _player_sprite;
}

uint8_t player_is_facing_right(void)
{
    return _player_facing_r;
}

uint8_t player_is_jumping(void)
{
    return (_player_state == PSTATE_JUMP) ? 1U : 0U;
}
