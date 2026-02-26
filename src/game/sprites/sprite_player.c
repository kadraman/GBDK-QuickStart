#include <gb/gb.h>
#include <gb/cgb.h>
#include <stddef.h>
#include <stdint.h>
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_player.h"
#include "player.h"

/* -----------------------------------------------------------------------
 * Player physics constants
 * -------------------------------------------------------------------- */
#define JUMP_VY          (-6)   /* initial jump velocity (negative = up) */
#define WALK_SPEED         1U   /* world pixels per frame                */
#define GRAVITY_DELAY      3U   /* frames between gravity steps          */

/* World extents – 48-tile map */
#define MAX_WORLD_X      376U   /* 48*8-8 = 376                          */
#define SCROLL_R_LIMIT   100U   /* scroll right when screen-X exceeds    */
#define SCROLL_L_LIMIT    60U   /* scroll left  when screen-X falls below */
#define MAX_SCROLL_X     224U   /* max camera_x: (48-20)*8 = 224         */

/* Sprite Y constants (world_y = screen top of sprite; OAM Y = world_y+16) */
#define GROUND_WORLD_Y    64U   /* world_y standing on ground floor      */
#define PLATFORM_WORLD_Y  40U   /* world_y standing on a raised platform */
#define MAX_FALL_WORLD_Y 160U   /* world_y at which player is dead       */

/* -----------------------------------------------------------------------
 * Pit zones: world-x ranges with no ground floor.
 * Matches the pit definitions in res/backgrounds/gameplay/definition.py:
 *   PIT1: cols 10-12  => world-x  80..103
 *   PIT2: cols 21-24  => world-x 168..199
 *   PIT3: cols 34-38  => world-x 272..311
 * -------------------------------------------------------------------- */
static const uint16_t _pits[][2] = {
    { 80U, 103U},
    {168U, 199U},
    {272U, 311U}
};
#define NUM_PITS  3U

/* -----------------------------------------------------------------------
 * Platform zones: world-x ranges with a raised platform.
 * Matches the platform definitions in res/backgrounds/gameplay/definition.py:
 *   PLAT1: col 7      => world-x  56..63
 *   PLAT2: cols 15-16 => world-x 120..135
 *   PLAT3: cols 27-28 => world-x 216..231
 *   PLAT4: col 42     => world-x 336..343
 * -------------------------------------------------------------------- */
static const uint16_t _platforms[][2] = {
    { 56U,  63U},
    {120U, 135U},
    {216U, 231U},
    {336U, 343U},
};
#define NUM_PLATFORMS  4U

typedef enum { PSTATE_IDLE, PSTATE_WALK, PSTATE_JUMP } PlayerState;

static Sprite      *_player_sprite;
static uint16_t     _player_world_x16;   /* full 16-bit world X position  */
static int8_t       _player_vy;
static uint8_t      _player_facing_r;
static PlayerState  _player_state;
static uint8_t      _on_platform;
static uint8_t      _gravity_delay_ctr;

/* Returns 1 if world_x is over a pit (no ground). */
static uint8_t _over_pit(uint16_t wx)
{
    uint8_t i;
    for (i = 0U; i < NUM_PITS; i++) {
        if (wx >= _pits[i][0] && wx <= _pits[i][1]) {
            return 1U;
        }
    }
    return 0U;
}

/* Returns 1 if world_x is over a platform, 0 otherwise. */
static uint8_t _over_platform(uint16_t wx)
{
    uint8_t i;
    for (i = 0U; i < NUM_PLATFORMS; i++) {
        if (wx >= _platforms[i][0] && wx <= _platforms[i][1]) {
            return 1U;
        }
    }
    return 0U;
}

void player_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base)
{
    _player_vy         = 0;
    _player_facing_r   = 1U;
    _player_state      = PSTATE_IDLE;
    _on_platform       = 0U;
    _gravity_delay_ctr = 0U;
    _player_world_x16  = (uint16_t)start_x;

    _player_sprite = sprite_manager_alloc(
        0U, 2U, 8U, 16U, tile_base, PLAYER_TILES_PER_FRAME);
    _player_sprite->world_x    = start_x;
    _player_sprite->world_y    = ground_y;
    _player_sprite->anim_speed = PLAYER_ANIM_IDLE_SPEED;

    set_sprite_tile(0U, PLAYER_ANIM_IDLE_START);
    set_sprite_tile(1U, (uint8_t)(PLAYER_ANIM_IDLE_START + 2U));
    set_sprite_prop(0U, 0U);
    set_sprite_prop(1U, 0U);
    sprite_manager_update_hw(_player_sprite, 0U, 0U);
}

uint8_t player_update(uint8_t joy, uint8_t joy_press, uint8_t *camera_x,
                      uint16_t min_world_x)
{
    uint8_t  events  = 0U;
    uint8_t  moved   = 0U;
    uint16_t new_x16;
    int16_t  new_y;
    uint8_t  screen_x, hw_x;
    uint8_t  tile_idx, prop;
    uint8_t  anim_start, anim_frames;
    uint8_t  over_pit, over_plat;

    /* --- Horizontal movement --- */
    if (joy & J_RIGHT) {
        _player_facing_r = 1U;
        if (_player_world_x16 < (uint16_t)MAX_WORLD_X) {
            new_x16 = (uint16_t)(_player_world_x16 + WALK_SPEED);
            _player_world_x16 = new_x16;
            moved = 1U;
        }
    } else if (joy & J_LEFT) {
        _player_facing_r = 0U;
        if (_player_world_x16 > min_world_x) {
            new_x16 = (uint16_t)(_player_world_x16 - WALK_SPEED);
            _player_world_x16 = new_x16;
            moved = 1U;
        }
    }

    /* Update sprite world_x with screen-space value (for collision) */
    _player_sprite->world_x =
        (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x));

    /* --- Jump (A or B button, only when grounded / on platform) --- */
    if ((joy_press & J_A) || (joy_press & J_B)) {
        if (_player_state != PSTATE_JUMP) {
            _player_vy         = JUMP_VY;
            _gravity_delay_ctr = 0U;
            _player_state      = PSTATE_JUMP;
            _on_platform       = 0U;
            events |= PLAYER_EVENT_JUMPED;
        }
    }

    /* --- Determine current floor / platform --- */
    over_pit  = _over_pit(_player_world_x16);
    over_plat = _on_platform ? _over_platform(_player_world_x16) : 0U;

    /* --- Grounded on solid floor but walked into pit: start falling --- */
    if (_player_state != PSTATE_JUMP && !_on_platform && over_pit) {
        _player_state      = PSTATE_JUMP;
        _player_vy         = 0;
        _gravity_delay_ctr = 0U;
    }

    /* --- On platform but walked off edge: start falling --- */
    if (_on_platform && !over_plat) {
        _on_platform       = 0U;
        _player_state      = PSTATE_JUMP;
        _player_vy         = 0;
        _gravity_delay_ctr = 0U;
    }

    /* --- Vertical physics --- */
    if (_player_state == PSTATE_JUMP) {
        new_y = (int16_t)_player_sprite->world_y + _player_vy;
        if (new_y < 0) new_y = 0;

        /* Platform landing (only when falling downward) */
        if (_player_vy > 0 && _over_platform(_player_world_x16)) {
            if ((int16_t)_player_sprite->world_y <= (int16_t)PLATFORM_WORLD_Y &&
                new_y >= (int16_t)PLATFORM_WORLD_Y) {
                new_y              = (int16_t)PLATFORM_WORLD_Y;
                _player_vy         = 0;
                _gravity_delay_ctr = 0U;
                _player_state      = moved ? PSTATE_WALK : PSTATE_IDLE;
                _on_platform       = 1U;
            }
        }

        /* Ground landing (not over a pit) */
        if (_player_state == PSTATE_JUMP && _player_vy >= 0 && !over_pit) {
            if (new_y >= (int16_t)GROUND_WORLD_Y) {
                new_y              = (int16_t)GROUND_WORLD_Y;
                _player_vy         = 0;
                _gravity_delay_ctr = 0U;
                _player_state      = moved ? PSTATE_WALK : PSTATE_IDLE;
                _on_platform       = 0U;
            }
        }

        /* Apply gravity every GRAVITY_DELAY frames */
        if (_player_state == PSTATE_JUMP) {
            _gravity_delay_ctr++;
            if (_gravity_delay_ctr >= GRAVITY_DELAY) {
                _gravity_delay_ctr = 0U;
                _player_vy++;
            }
        }

        _player_sprite->world_y = (uint8_t)new_y;

        /* Fell off the bottom: signal event */
        if (_player_sprite->world_y >= MAX_FALL_WORLD_Y) {
            events |= PLAYER_EVENT_FELL_GAP;
        }
    } else {
        /* On ground or platform: maintain correct Y */
        _player_sprite->world_y = _on_platform ? PLATFORM_WORLD_Y : GROUND_WORLD_Y;

        PlayerState next = moved ? PSTATE_WALK : PSTATE_IDLE;
        if (next != _player_state) {
            _player_state                  = next;
            _player_sprite->anim_frame     = 0U;
            _player_sprite->anim_counter   = 0U;
            _player_sprite->anim_speed     = (next == PSTATE_WALK)
                                             ? PLAYER_ANIM_WALK_SPEED
                                             : PLAYER_ANIM_IDLE_SPEED;
        }
    }

    /* --- Camera / scroll --- */
    screen_x = (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x));
    if (screen_x > SCROLL_R_LIMIT && *camera_x < MAX_SCROLL_X) {
        (*camera_x)++;
    } else if (screen_x < SCROLL_L_LIMIT && *camera_x > 0U) {
        (*camera_x)--;
    }
    SCX_REG = *camera_x;

    /* Re-sync sprite world_x with updated camera */
    _player_sprite->world_x =
        (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x));

    /* --- Animation selection --- */
    if (_player_state == PSTATE_JUMP) {
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
    hw_x = (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x) + 8U);
    move_sprite(0U, hw_x, (uint8_t)(_player_sprite->world_y + 16U));
    move_sprite(1U, (uint8_t)(hw_x + 8U), (uint8_t)(_player_sprite->world_y + 16U));

    return events;
}

void player_cleanup(void)
{
    if (_player_sprite) {
        sprite_manager_free(_player_sprite);
        _player_sprite = NULL;
    }
}

Sprite* player_get_sprite(void)
{
    return _player_sprite;
}

uint16_t player_get_world_x16(void)
{
    return _player_world_x16;
}

uint8_t player_is_facing_right(void)
{
    return _player_facing_r;
}

uint8_t player_is_jumping(void)
{
    return (_player_state == PSTATE_JUMP) ? 1U : 0U;
}
