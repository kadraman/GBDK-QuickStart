#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "sprite.h"
#include "sprite_manager.h"
#include "sprite_enemy.h"
#include "../res/player.h"
#include "../res/enemy.h"

/* -----------------------------------------------------------------------
 * Enemy constants
 * -------------------------------------------------------------------- */
#define ENEMY_OBJ_ID          2U   /* hardware OBJ slot used by enemy     */
#define ENEMY_PATROL_LEFT    10U   /* left patrol boundary (world-X)      */
#define ENEMY_PATROL_RIGHT  180U   /* right patrol boundary (world-X)     */

static Sprite  *_enemy_sprite;
static int8_t   _enemy_dx;        /* patrol direction (+1 or -1)         */
static uint8_t  _enemy_is_idle;   /* 1 = currently using idle animation  */

void enemy_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base)
{
    _enemy_dx     = 1;
    _enemy_is_idle = 0U;

    _enemy_sprite = sprite_manager_alloc(
        ENEMY_OBJ_ID, 1U, 8U, 8U, tile_base, ENEMY_TILES_PER_FRAME);
    _enemy_sprite->world_x    = start_x;
    _enemy_sprite->world_y    = ground_y;
    _enemy_sprite->anim_speed = ENEMY_ANIM_WALK_SPEED;

    /* GBC sprite palette slot 1 for enemy */
    set_sprite_tile(ENEMY_OBJ_ID, (uint8_t)(tile_base + ENEMY_ANIM_WALK_START));
    set_sprite_prop(ENEMY_OBJ_ID, 0x01U);
    sprite_manager_update_hw(_enemy_sprite, 0U, 0U);
}

void enemy_update(uint8_t camera_x)
{
    uint8_t tile_idx;
    uint8_t prop;
    uint8_t anim_start, anim_frames;

    /* --- Patrol movement --- */
    _enemy_sprite->world_x = (uint8_t)((int16_t)_enemy_sprite->world_x + _enemy_dx);
    if (_enemy_sprite->world_x >= ENEMY_PATROL_RIGHT) { _enemy_dx = -1; }
    if (_enemy_sprite->world_x <= ENEMY_PATROL_LEFT)  { _enemy_dx =  1; }

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

    /* --- Move OBJ hardware slot --- */
    sprite_manager_update_hw(_enemy_sprite, camera_x, 0U);
}

void enemy_cleanup(void)
{
    if (_enemy_sprite) {
        sprite_manager_free(_enemy_sprite);
        _enemy_sprite = 0;
    }
}

Sprite* enemy_get_sprite(void)
{
    return _enemy_sprite;
}
