#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

/* -----------------------------------------------------------------------
 * Sprite – base structure shared by all game sprites.
 *
 * Coordinate conventions
 * ----------------------
 *   world_x   : horizontal world-space pixel position (0..MAX_WORLD_X)
 *   hw_y      : OAM Y register value; screen top = hw_y - 16
 *   width     : collision / logical width in pixels  (8 or 16)
 *   height    : visual height in pixels              (8 or 16)
 *
 * Hardware sprite slots
 * ---------------------
 *   obj_id    : first GBDK sprite (OBJ) slot used
 *   num_objs  : number of consecutive OBJ slots (1 for 8x8/8x16, 2 for 16x16)
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
 *
 * Lifecycle
 * ---------
 *   active : 1 = sprite is active and visible; 0 = inactive / hidden
 * ----------------------------------------------------------------------- */
typedef struct {
    uint8_t  obj_id;           /* first hardware OBJ slot                  */
    uint8_t  num_objs;         /* OBJ slots used (1 or 2)                  */
    uint8_t  world_x;          /* world-space X position                   */
    uint8_t  hw_y;             /* OAM Y register value                     */
    uint8_t  width;            /* logical/collision width in pixels        */
    uint8_t  height;           /* visual height in pixels                  */
    uint8_t  tile_base;        /* first VRAM tile index for this sprite    */
    uint8_t  tiles_per_frame;  /* tiles per animation frame                */
    uint8_t  anim_frame;       /* current animation frame                  */
    uint8_t  anim_counter;     /* vblanks elapsed in current frame         */
    uint8_t  active;           /* 1 = active/visible, 0 = hidden           */
} Sprite;

/* -----------------------------------------------------------------------
 * sprites_collide
 *
 * AABB collision test between two sprites in world + screen space.
 * Returns 1 if the sprites overlap, 0 otherwise.
 *
 * X-axis uses world_x and width.
 * Y-axis uses hw_y and height: screen_top = hw_y - 16, screen_bot = hw_y - 16 + height.
 * ----------------------------------------------------------------------- */
uint8_t sprites_collide(const Sprite *a, const Sprite *b);

#endif
