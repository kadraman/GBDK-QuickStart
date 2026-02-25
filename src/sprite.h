#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

/* -----------------------------------------------------------------------
 * Sprite – base structure shared by all game sprites.
 *
 * Coordinate conventions
 * ----------------------
 *   world_x   : horizontal world-space pixel position (0..MAX_WORLD_X)
 *   world_y   : vertical world-space pixel position; screen top of sprite.
 *               OAM Y register = world_y + 16  (same formula works for
 *               vertical scrolling: OAM Y = world_y - camera_y + 16).
 *   width     : visual width in pixels  (8 or 16)
 *   height    : visual height in pixels (8 or 16)
 *
 * Collision box
 * -------------
 *   hitbox_x / hitbox_y : offset from world_x/world_y to top-left of hitbox
 *   hitbox_w / hitbox_h : hitbox size in pixels (0 = use full width/height)
 *
 * Hardware sprite slots
 * ---------------------
 *   obj_id    : first GBDK sprite (OBJ) slot used
 *   num_objs  : number of consecutive OBJ slots (1 for 8x8/8x16, 2 for 16x16)
 *               NOTE: num_objs is the pool-slot count, not the OBJ-slot count.
 *               A 16x16 sprite occupies 1 pool slot but 2 hardware OBJ slots.
 *
 * Tile data
 * ---------
 *   tile_base       : first VRAM tile slot for this sprite's tile data
 *   tiles_per_frame : tiles consumed per animation frame
 *
 * Animation
 * ---------
 *   anim_frame   : current frame index within the active animation
 *   anim_counter : frame-timer counter (vblanks elapsed in current frame)
 *   anim_speed   : vblanks per animation frame (set per animation)
 *
 * Lifecycle
 * ---------
 *   active : 1 = sprite is active and visible; 0 = inactive / hidden
 *
 * Custom data
 * -----------
 *   custom_data : 4 bytes of user-defined per-sprite state (flags, counters,
 *                 IDs, etc.) available for custom sprite behaviours.
 * ----------------------------------------------------------------------- */
typedef struct {
    uint8_t  obj_id;           /* first hardware OBJ slot                  */
    uint8_t  num_objs;         /* hardware OBJ slots used (1 or 2)         */
    uint8_t  world_x;          /* world-space X position                   */
    uint8_t  world_y;          /* world-space Y position (screen top)      */
    uint8_t  width;            /* visual width in pixels                   */
    uint8_t  height;           /* visual height in pixels                  */
    uint8_t  hitbox_x;         /* hitbox X offset from world_x             */
    uint8_t  hitbox_y;         /* hitbox Y offset from world_y             */
    uint8_t  hitbox_w;         /* hitbox width  (0 = use full width)       */
    uint8_t  hitbox_h;         /* hitbox height (0 = use full height)      */
    uint8_t  tile_base;        /* first VRAM tile index for this sprite    */
    uint8_t  tiles_per_frame;  /* tiles per animation frame                */
    uint8_t  anim_frame;       /* current animation frame                  */
    uint8_t  anim_counter;     /* vblanks elapsed in current frame         */
    uint8_t  anim_speed;       /* vblanks per animation frame              */
    uint8_t  active;           /* 1 = active/visible, 0 = hidden           */
    uint8_t  custom_data[4];   /* user-defined per-sprite data              */
} Sprite;

/* -----------------------------------------------------------------------
 * sprites_collide
 *
 * AABB collision test between two sprites.
 * Uses hitbox_x/y/w/h if set; otherwise falls back to full sprite bounds.
 * Returns 1 if the sprites overlap, 0 otherwise.
 * ----------------------------------------------------------------------- */
uint8_t sprites_collide(const Sprite *a, const Sprite *b);

#endif
