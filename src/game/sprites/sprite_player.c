#include <gb/gb.h>
#include <gb/cgb.h>
#include <stddef.h>
#include <stdint.h>
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_player.h"
#include "player.h"
#include "bg_gameplay.h"

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

/* Sprite Y constants */
#define GROUND_WORLD_Y    64U   /* initial world_y when spawned on ground */
#define MAX_FALL_WORLD_Y 160U   /* world_y at which player is considered dead */

typedef enum { PSTATE_IDLE, PSTATE_WALK, PSTATE_JUMP } PlayerState;

static Sprite      *_player_sprite;
static uint16_t     _player_world_x16;   /* full 16-bit world X position  */
static int8_t       _player_vy;
static uint8_t      _player_facing_r;
static PlayerState  _player_state;
static uint8_t      _gravity_delay_ctr;

/* -----------------------------------------------------------------------
 * _has_ground_below
 * Returns 1 if there is a collideable tile directly beneath the player's
 * feet.  The tile row is derived dynamically from world_y + sprite height,
 * so this works correctly at any elevation (ground or platform).
 * -------------------------------------------------------------------- */
static uint8_t _has_ground_below(uint8_t world_y, uint16_t world_x16)
{
    uint8_t feet_y;
    uint8_t tile_row;
    uint8_t tile;
    uint8_t i;

    feet_y   = (uint8_t)(world_y + _player_sprite->height);
    tile_row = (uint8_t)(feet_y >> 3);
    tile     = sprite_manager_tile_at(
        world_x16, tile_row, bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH);
    for (i = 0U; i < BG_GAMEPLAY_COLLISION_TILE_COUNT; i++) {
        if (bg_gameplay_collision_tiles[i] == tile) return 1U;
    }
    return 0U;
}

void player_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base)
{
    _player_vy         = 0;
    _player_facing_r   = 1U;
    _player_state      = PSTATE_IDLE;
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
    uint8_t     events  = 0U;
    uint8_t     moved   = 0U;
    int16_t     new_y;
    uint8_t     screen_x, hw_x;
    uint8_t     tile_idx, prop;
    uint8_t     anim_start, anim_frames;
    uint8_t     snap_row;
    PlayerState next;

    /* --- Horizontal movement with solid-tile wall collision --- */
    if (joy & J_RIGHT) {
        _player_facing_r = 1U;
        if (_player_world_x16 < (uint16_t)MAX_WORLD_X) {
            _player_world_x16++;
            _player_sprite->world_x =
                (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x));
            /* Check for wall: solid tiles block left/right/above/below */
            if (sprite_manager_tile_collision(
                    _player_sprite, _player_world_x16,
                    bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH, BG_GAMEPLAY_MAP_HEIGHT,
                    bg_gameplay_solid_tiles, BG_GAMEPLAY_SOLID_TILE_COUNT)) {
                _player_world_x16--;   /* revert */
            } else {
                moved = 1U;
            }
        }
    } else if (joy & J_LEFT) {
        _player_facing_r = 0U;
        if (_player_world_x16 > min_world_x) {
            _player_world_x16--;
            _player_sprite->world_x =
                (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x));
            if (sprite_manager_tile_collision(
                    _player_sprite, _player_world_x16,
                    bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH, BG_GAMEPLAY_MAP_HEIGHT,
                    bg_gameplay_solid_tiles, BG_GAMEPLAY_SOLID_TILE_COUNT)) {
                _player_world_x16++;   /* revert */
            } else {
                moved = 1U;
            }
        }
    }

    /* Sync screen-relative world_x after potential revert */
    _player_sprite->world_x =
        (uint8_t)(_player_world_x16 - (uint16_t)(*camera_x));

    /* --- Jump (A or B button, only when grounded) --- */
    if ((joy_press & J_A) || (joy_press & J_B)) {
        if (_player_state != PSTATE_JUMP) {
            _player_vy         = JUMP_VY;
            _gravity_delay_ctr = 0U;
            _player_state      = PSTATE_JUMP;
            events |= PLAYER_EVENT_JUMPED;
        }
    }

    /* --- Walking off an edge: start falling when no collideable tile below --- */
    if (_player_state != PSTATE_JUMP &&
        !_has_ground_below(_player_sprite->world_y, _player_world_x16)) {
        _player_state      = PSTATE_JUMP;
        _player_vy         = 0;
        _gravity_delay_ctr = 0U;
    }

    /* --- Vertical physics --- */
    if (_player_state == PSTATE_JUMP) {
        new_y = (int16_t)_player_sprite->world_y + _player_vy;
        if (new_y < 0) new_y = 0;

        _player_sprite->world_y = (uint8_t)new_y;

        /* Landing from above: collideable tiles (includes one-way platforms) */
        if (_player_vy >= 0 &&
            sprite_manager_tile_collision(
                _player_sprite, _player_world_x16,
                bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH, BG_GAMEPLAY_MAP_HEIGHT,
                bg_gameplay_collision_tiles, BG_GAMEPLAY_COLLISION_TILE_COUNT)) {
            /* Snap player to the top of the tile the feet entered */
            snap_row = (uint8_t)(((uint8_t)new_y + _player_sprite->height) >> 3);
            new_y = (int16_t)(snap_row * 8U) - (int16_t)_player_sprite->height;
            _player_sprite->world_y = (uint8_t)new_y;
            _player_vy         = 0;
            _gravity_delay_ctr = 0U;
            _player_state      = moved ? PSTATE_WALK : PSTATE_IDLE;
        }

        /* Ceiling: solid tiles block upward movement */
        if (_player_vy < 0 &&
            sprite_manager_tile_collision(
                _player_sprite, _player_world_x16,
                bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH, BG_GAMEPLAY_MAP_HEIGHT,
                bg_gameplay_solid_tiles, BG_GAMEPLAY_SOLID_TILE_COUNT)) {
            /* Snap to the bottom of the tile hit from below */
            snap_row = (uint8_t)((uint8_t)new_y >> 3);
            _player_sprite->world_y = (uint8_t)((snap_row + 1U) * 8U);
            _player_vy = 1;   /* start falling */
        }

        /* Apply gravity every GRAVITY_DELAY frames */
        if (_player_state == PSTATE_JUMP) {
            _gravity_delay_ctr++;
            if (_gravity_delay_ctr >= GRAVITY_DELAY) {
                _gravity_delay_ctr = 0U;
                _player_vy++;
            }
        }

        /* Fell off the bottom: signal event */
        if (_player_sprite->world_y >= MAX_FALL_WORLD_Y) {
            events |= PLAYER_EVENT_FELL_GAP;
        }
    } else {
        /* On ground or platform: handle idle/walk state transitions */
        next = moved ? PSTATE_WALK : PSTATE_IDLE;
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
