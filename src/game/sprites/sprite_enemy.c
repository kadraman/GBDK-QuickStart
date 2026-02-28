#include <gb/gb.h>
#include <gb/cgb.h>
#include <stddef.h>
#include <stdint.h>
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_enemy.h"
#include "player.h"
#include "enemy.h"
#include "bg_gameplay.h"

/* -----------------------------------------------------------------------
 * Enemy constants
 * -------------------------------------------------------------------- */
#define ENEMY_OBJ_ID          2U   /* hardware OBJ slot used by enemy     */
#define ENEMY_PATROL_LEFT    10U   /* left patrol boundary (world-X)      */
#define ENEMY_PATROL_RIGHT  180U   /* right patrol boundary (world-X)     */

static Sprite   *_enemy_sprite;
static uint16_t  _enemy_world_x16;   /* full 16-bit absolute world X      */
static int8_t    _enemy_dx;          /* patrol direction (+1 or -1)       */
static uint8_t   _enemy_is_idle;     /* 1 = currently using idle animation */

/* -----------------------------------------------------------------------
 * _enemy_has_ground_at
 * Returns 1 if there is a solid tile (ground) at the given world-X and
 * directly below the enemy's feet.  Used to detect pit edges.
 * -------------------------------------------------------------------- */
static uint8_t _enemy_has_ground_at(uint16_t world_x16)
{
    uint8_t feet_y;
    uint8_t tile_row;
    uint8_t tile;
    uint8_t i;

    feet_y   = (uint8_t)(_enemy_sprite->world_y + _enemy_sprite->height);
    tile_row = (uint8_t)(feet_y >> 3);
    tile     = sprite_manager_tile_at(
        world_x16, tile_row, bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH);
    for (i = 0U; i < BG_GAMEPLAY_SOLID_TILE_COUNT; i++) {
        if (bg_gameplay_solid_tiles[i] == tile) return 1U;
    }
    return 0U;
}

void enemy_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base)
{
    _enemy_dx      = 1;
    _enemy_is_idle = 0U;
    _enemy_world_x16 = (uint16_t)start_x;

    _enemy_sprite = sprite_manager_alloc(
        ENEMY_OBJ_ID, 1U, 8U, 8U, tile_base, ENEMY_TILES_PER_FRAME);
    _enemy_sprite->world_x    = start_x;
    _enemy_sprite->world_y    = ground_y;
    _enemy_sprite->anim_speed = ENEMY_ANIM_WALK_SPEED;

    /* GBC sprite palette slot 1 for enemy */
    set_sprite_tile(ENEMY_OBJ_ID, (uint8_t)(tile_base + ENEMY_ANIM_WALK_START));
    set_sprite_prop(ENEMY_OBJ_ID, 0x01U);
    move_sprite(ENEMY_OBJ_ID, (uint8_t)(start_x + 8U),
                (uint8_t)(_enemy_sprite->world_y + 16U));
}

void enemy_update(uint8_t camera_x)
{
    uint16_t next_x16;
    int16_t  screen_x;
    uint8_t  hw_x, hw_y;
    uint8_t  tile_idx;
    uint8_t  prop;
    uint8_t  anim_start, anim_frames;

    /* --- Patrol movement with pit-edge and wall detection --- */
    next_x16 = (uint16_t)((int16_t)_enemy_world_x16 + _enemy_dx);

    /* Check for solid tile wall ahead and solid ground below next step */
    _enemy_sprite->world_x = (uint8_t)next_x16;  /* temp for tile_collision */
    if (sprite_manager_tile_collision(
            _enemy_sprite, next_x16,
            bg_gameplay_map, BG_GAMEPLAY_MAP_WIDTH, BG_GAMEPLAY_MAP_HEIGHT,
            bg_gameplay_solid_tiles, BG_GAMEPLAY_SOLID_TILE_COUNT) ||
        !_enemy_has_ground_at(next_x16)) {
        /* Hit a wall or about to walk off a pit edge – reverse direction */
        _enemy_dx = -_enemy_dx;
        next_x16  = _enemy_world_x16;   /* stay in place this frame */
    }

    _enemy_world_x16 = next_x16;

    /* Enforce patrol boundaries */
    if (_enemy_world_x16 >= (uint16_t)ENEMY_PATROL_RIGHT) {
        _enemy_dx = -1;
    }
    if (_enemy_world_x16 <= (uint16_t)ENEMY_PATROL_LEFT) {
        _enemy_dx = 1;
    }

    /* --- Animation: walk while moving --- */
    anim_start  = ENEMY_ANIM_WALK_START;
    anim_frames = ENEMY_ANIM_WALK_FRAMES;
    if (_enemy_is_idle) {
        anim_start  = ENEMY_ANIM_IDLE_START;
        anim_frames = ENEMY_ANIM_IDLE_FRAMES;
    }

    _enemy_sprite->anim_counter++;
    if (_enemy_sprite->anim_counter >= _enemy_sprite->anim_speed) {
        _enemy_sprite->anim_counter = 0U;
        _enemy_sprite->anim_frame   =
            (uint8_t)((_enemy_sprite->anim_frame + 1U) % anim_frames);
    }
    tile_idx = (uint8_t)(_enemy_sprite->tile_base + anim_start +
                          _enemy_sprite->anim_frame * ENEMY_TILES_PER_FRAME);
    set_sprite_tile(ENEMY_OBJ_ID, tile_idx);

    /* --- Flip enemy to face direction of travel --- */
    prop = get_sprite_prop(ENEMY_OBJ_ID);
    if (_enemy_dx < 0) {
        prop = (uint8_t)(prop | S_FLIPX);
    } else {
        prop = (uint8_t)(prop & ~S_FLIPX);
    }
    set_sprite_prop(ENEMY_OBJ_ID, prop);

    /* --- Compute screen-relative X using signed arithmetic ---
     * This fixes the "enemy appears on wrong side" bug when camera has
     * scrolled past the enemy's world position.
     * world_x is kept screen-relative so sprites_collide() correctly
     * compares player (screen-relative) vs enemy (screen-relative).   */
    screen_x = (int16_t)_enemy_world_x16 - (int16_t)camera_x;
    _enemy_sprite->world_x = (uint8_t)screen_x;

    if (screen_x < -8 || screen_x > 168) {
        /* Enemy is off-screen: hide the hardware sprite */
        move_sprite(ENEMY_OBJ_ID, 0U, 0U);
    } else {
        hw_x = (uint8_t)(screen_x + 8);
        hw_y = (uint8_t)(_enemy_sprite->world_y + 16U);
        move_sprite(ENEMY_OBJ_ID, hw_x, hw_y);
    }
}

void enemy_cleanup(void)
{
    if (_enemy_sprite) {
        sprite_manager_free(_enemy_sprite);
        _enemy_sprite = NULL;
    }
}

Sprite* enemy_get_sprite(void)
{
    return _enemy_sprite;
}
