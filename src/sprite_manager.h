#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "sprite.h"
#include <stdint.h>

/* Maximum number of concurrently managed sprites. */
#define SPRITE_MANAGER_MAX  8U

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
 * num_objs        : OBJ slots needed (1 for 8x8/8x16, 2 for 16x16)
 * width / height  : logical/collision dimensions in pixels
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
 * Move the sprite's OBJ slot(s) to match world_x / hw_y, accounting for
 * the camera offset.  For 16x16 sprites (num_objs == 2) the second OBJ is
 * placed 8 pixels to the right of the first.
 * ----------------------------------------------------------------------- */
void sprite_manager_update_hw(const Sprite *s, uint8_t camera_x);

#endif
