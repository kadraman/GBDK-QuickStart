#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "sprite.h"
#include <stdint.h>

/* Maximum number of concurrently managed logical sprites.
 *
 * Each pool slot holds one Sprite struct (one logical sprite).  A 16x16
 * sprite uses 1 pool slot but 2 hardware OBJ slots; an 8x8/8x16 sprite
 * uses 1 pool slot and 1 hardware OBJ slot.  The GBC has 40 hardware OBJ
 * slots total.  16 pool slots is a good balance for typical games. */
#define SPRITE_MANAGER_MAX  16U

/* -----------------------------------------------------------------------
 * sprite_manager_init
 * Mark all pool slots as inactive.  Call once at the start of each state
 * that uses sprites.
 * ----------------------------------------------------------------------- */
void sprite_manager_init(void);

/* -----------------------------------------------------------------------
 * sprite_manager_alloc
 * Claim a free pool slot and initialise it with the given parameters.
 * Returns a pointer to the Sprite on success, or NULL if the pool is full.
 *
 * obj_id          : first GBDK OBJ slot assigned to this sprite
 * num_objs        : number of OBJ slots needed (1 for 8x8/8x16, 2 for 16x16)
 * width / height  : visual/collision dimensions in pixels
 * tile_base       : first VRAM tile index for this sprite's tile data
 * tiles_per_frame : tiles consumed per animation frame
 * ----------------------------------------------------------------------- */
Sprite* sprite_manager_alloc(uint8_t obj_id,
                              uint8_t num_objs,
                              uint8_t width,
                              uint8_t height,
                              uint8_t tile_base,
                              uint8_t tiles_per_frame);

/* -----------------------------------------------------------------------
 * sprite_manager_free
 * Return a sprite to the pool and hide its OBJ slot(s).
 * Passing NULL is safe (no-op).
 * ----------------------------------------------------------------------- */
void sprite_manager_free(Sprite *s);

/* -----------------------------------------------------------------------
 * sprite_manager_update_hw
 * Move the sprite's OBJ slot(s) to match world_x / world_y, accounting for
 * the camera offsets.  OAM X = world_x - camera_x + 8;
 * OAM Y = world_y - camera_y + 16.
 * For 16x16 sprites (num_objs == 2) the second OBJ is placed 8 pixels to
 * the right of the first.
 * ----------------------------------------------------------------------- */
void sprite_manager_update_hw(const Sprite *s, uint8_t camera_x, uint8_t camera_y);

/* -----------------------------------------------------------------------
 * sprite_manager_first_collision
 * Iterate over all active pool slots (excluding s itself) and return the
 * first sprite that collides with s (AABB test via sprites_collide).
 * Returns NULL if no collision is found.
 * Use this to check whether a sprite (e.g. the player) has hit any enemy.
 * ----------------------------------------------------------------------- */
Sprite* sprite_manager_first_collision(const Sprite *s);

/* -----------------------------------------------------------------------
 * sprite_manager_tile_at
 * Look up the tile ID at a world-pixel X and tile-row Y in a ROM tilemap.
 *
 * world_x16 : full 16-bit world X coordinate in pixels
 * tile_row  : map row index (0 = top of map)
 * tilemap   : flat row-major tilemap array (stored in ROM)
 * map_width : map width in tiles
 *
 * Returns the tile ID, or 0 if the column is out of bounds.
 * ----------------------------------------------------------------------- */
uint8_t sprite_manager_tile_at(uint16_t world_x16, uint8_t tile_row,
                                const uint8_t *tilemap, uint8_t map_width);

/* -----------------------------------------------------------------------
 * sprite_manager_tile_collision
 * Check whether a sprite's AABB overlaps any tile whose ID appears in the
 * collide_tiles list.  Uses hitbox_x/y/w/h if set; otherwise falls back
 * to the full sprite dimensions.
 *
 * world_x16        : full 16-bit world X of the sprite's left edge
 * tilemap          : flat row-major tilemap array (stored in ROM)
 * map_width        : map width in tiles
 * map_height       : map height in tiles
 * collide_tiles    : array of tile IDs treated as solid/collideable
 * num_collide_tiles: number of entries in collide_tiles
 *
 * Returns 1 if the sprite overlaps a collideable tile, 0 otherwise.
 * ----------------------------------------------------------------------- */
uint8_t sprite_manager_tile_collision(const Sprite  *s,
                                       uint16_t       world_x16,
                                       const uint8_t *tilemap,
                                       uint8_t        map_width,
                                       uint8_t        map_height,
                                       const uint8_t *collide_tiles,
                                       uint8_t        num_collide_tiles);

#endif
