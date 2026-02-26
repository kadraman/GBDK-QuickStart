#ifndef SPRITE_PLAYER_H
#define SPRITE_PLAYER_H

#include <stdint.h>
#include "sprite.h"

/* -----------------------------------------------------------------------
 * Player sprite – provides init, update, and cleanup for the player.
 *
 * Call player_init() once when entering the gameplay state.
 * Call player_update() every frame (after vsync).
 * Call player_cleanup() when leaving the gameplay state.
 * ----------------------------------------------------------------------- */

/* Return value flags from player_update() */
#define PLAYER_EVENT_JUMPED    0x01U  /* player jumped this frame     */
#define PLAYER_EVENT_FELL_GAP  0x02U  /* player fell into a gap       */

/* Initialise and allocate the player sprite.
 * start_x   : starting world-X position
 * ground_y  : world-Y when standing on the ground (sprite top of ground frame)
 * tile_base : first VRAM tile slot used by player tile data */
void player_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base);

/* Update player for one frame.
 * joy       : current joypad state
 * joy_press : buttons newly pressed this frame (joy & ~prev_joy)
 * camera_x  : pointer to current camera X scroll value; updated in place
 * min_world_x : leftward movement limit (ring-buffer safety, world pixels)
 * Returns   : bitmask of PLAYER_EVENT_* flags */
uint8_t player_update(uint8_t joy, uint8_t joy_press, uint8_t *camera_x,
                      uint16_t min_world_x);

/* Free the player sprite and hide its OBJ slots. */
void player_cleanup(void);

/* Return a pointer to the player's Sprite struct (for collision checks). */
Sprite* player_get_sprite(void);

/* Return 1 if player is facing right, 0 if facing left. */
uint8_t player_is_facing_right(void);

/* Return 1 if the player is currently in the jump state. */
uint8_t player_is_jumping(void);

/* Return the player's full 16-bit world X position. */
uint16_t player_get_world_x16(void);

#endif
