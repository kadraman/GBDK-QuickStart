#include <gb/gb.h>
#include <stddef.h>
#include <stdint.h>
#include "sprite.h"
#include "sprite_manager.h"

static Sprite _pool[SPRITE_MANAGER_MAX];

void sprite_manager_init(void)
{
    uint8_t i;
    for (i = 0U; i < SPRITE_MANAGER_MAX; i++) {
        _pool[i].active = 0U;
    }
}

Sprite* sprite_manager_alloc(uint8_t obj_id,
                              uint8_t num_objs,
                              uint8_t width,
                              uint8_t height,
                              uint8_t tile_base,
                              uint8_t tiles_per_frame)
{
    uint8_t i;
    for (i = 0U; i < SPRITE_MANAGER_MAX; i++) {
        if (!_pool[i].active) {
            _pool[i].obj_id          = obj_id;
            _pool[i].num_objs        = num_objs;
            _pool[i].width           = width;
            _pool[i].height          = height;
            _pool[i].hitbox_x        = 0U;
            _pool[i].hitbox_y        = 0U;
            _pool[i].hitbox_w        = 0U;
            _pool[i].hitbox_h        = 0U;
            _pool[i].tile_base       = tile_base;
            _pool[i].tiles_per_frame = tiles_per_frame;
            _pool[i].world_x         = 0U;
            _pool[i].world_y         = 0U;
            _pool[i].anim_frame      = 0U;
            _pool[i].anim_counter    = 0U;
            _pool[i].anim_speed      = 8U;
            _pool[i].active          = 1U;
            _pool[i].custom_data[0]  = 0U;
            _pool[i].custom_data[1]  = 0U;
            _pool[i].custom_data[2]  = 0U;
            _pool[i].custom_data[3]  = 0U;
            return &_pool[i];
        }
    }
    return NULL;   /* pool full */
}

void sprite_manager_free(Sprite *s)
{
    uint8_t i;
    if (!s) return;
    s->active = 0U;
    for (i = 0U; i < s->num_objs; i++) {
        move_sprite((uint8_t)(s->obj_id + i), 0U, 0U);
    }
}

void sprite_manager_update_hw(const Sprite *s, uint8_t camera_x, uint8_t camera_y)
{
    uint8_t hw_x, hw_y;
    if (!s || !s->active) return;
    hw_x = (uint8_t)(s->world_x - camera_x + 8U);
    hw_y = (uint8_t)(s->world_y - camera_y + 16U);
    if (s->num_objs >= 2U) {
        /* 16x16: two side-by-side 8x16 OBJ slots */
        move_sprite(s->obj_id, hw_x, hw_y);
        move_sprite((uint8_t)(s->obj_id + 1U), (uint8_t)(hw_x + 8U), hw_y);
    } else {
        move_sprite(s->obj_id, hw_x, hw_y);
    }
}

Sprite* sprite_manager_first_collision(const Sprite *s)
{
    uint8_t i;
    if (!s || !s->active) return NULL;
    for (i = 0U; i < SPRITE_MANAGER_MAX; i++) {
        if (&_pool[i] == s) continue;
        if (sprites_collide(s, &_pool[i])) {
            return &_pool[i];
        }
    }
    return NULL;
}

uint8_t sprite_manager_tile_at(uint16_t world_x16, uint8_t tile_row,
                                const uint8_t *tilemap, uint8_t map_width)
{
    uint16_t col;
    if (!tilemap) return 0U;
    col = (uint16_t)(world_x16 >> 3);
    if (col >= (uint16_t)map_width) return 0U;
    return tilemap[(uint16_t)tile_row * map_width + (uint8_t)col];
}

uint8_t sprite_manager_tile_collision(const Sprite  *s,
                                       uint16_t       world_x16,
                                       const uint8_t *tilemap,
                                       uint8_t        map_width,
                                       uint8_t        map_height,
                                       const uint8_t *collide_tiles,
                                       uint8_t        num_collide_tiles)
{
    uint16_t ax16;
    uint8_t  ay, aw, ah;
    uint16_t col_start, col_end;
    uint8_t  row_start, row_end;
    uint8_t  c, r, i, tile;

    if (!s || !s->active || !tilemap || !collide_tiles ||
        num_collide_tiles == 0U || map_width == 0U || map_height == 0U)
        return 0U;

    ax16 = world_x16 + (uint16_t)s->hitbox_x;
    ay   = (uint8_t)(s->world_y + s->hitbox_y);
    aw   = s->hitbox_w ? s->hitbox_w : s->width;
    ah   = s->hitbox_h ? s->hitbox_h : s->height;

    col_start = (uint16_t)(ax16 >> 3);
    col_end   = (uint16_t)((ax16 + aw - 1U) >> 3);
    row_start = (uint8_t)(ay >> 3);
    row_end   = (uint8_t)((ay + ah - 1U) >> 3);

    if (col_start >= (uint16_t)map_width) return 0U;
    if (col_end   >= (uint16_t)map_width) col_end  = (uint16_t)(map_width  - 1U);
    if (row_end   >= map_height)          row_end  = (uint8_t)(map_height  - 1U);

    for (r = row_start; r <= row_end; r++) {
        for (c = (uint8_t)col_start; c <= (uint8_t)col_end; c++) {
            tile = tilemap[(uint16_t)r * map_width + c];
            for (i = 0U; i < num_collide_tiles; i++) {
                if (collide_tiles[i] == tile) return 1U;
            }
        }
    }
    return 0U;
}
