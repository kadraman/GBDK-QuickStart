#ifndef SPRITE_PLAYER_H
#define SPRITE_PLAYER_H

#include <gbdk/platform.h>
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
#define PLAYER_EVENT_DIED      0x04U  /* death animation completed    */

/* Initialise and allocate the player sprite.
 * start_x   : starting world-X position
 * ground_y  : world-Y when standing on the ground (sprite top of ground frame)
 * tile_base : first VRAM tile slot used by player tile data */
BANKREF_EXTERN(player_init)
void player_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base) BANKED;

/* Update player for one frame.
 * joy       : current joypad state
 * joy_press : buttons newly pressed this frame (joy & ~prev_joy)
 * camera_x  : pointer to current camera X scroll value; updated in place
 * min_world_x : leftward movement limit (ring-buffer safety, world pixels)
 * Returns   : bitmask of PLAYER_EVENT_* flags */
BANKREF_EXTERN(player_update)
uint8_t player_update(uint8_t joy, uint8_t joy_press, uint8_t *camera_x,
                      uint16_t min_world_x) BANKED;

/* Free the player sprite and hide its OBJ slots. */
BANKREF_EXTERN(player_cleanup)
void player_cleanup(void) BANKED;

/* Return a pointer to the player's Sprite struct (for collision checks). */
BANKREF_EXTERN(player_get_sprite)
Sprite* player_get_sprite(void) BANKED;

/* Return 1 if player is facing right, 0 if facing left. */
BANKREF_EXTERN(player_is_facing_right)
uint8_t player_is_facing_right(void) BANKED;

/* Return 1 if the player is currently in the jump state. */
BANKREF_EXTERN(player_is_jumping)
uint8_t player_is_jumping(void) BANKED;

/* Return the player's full 16-bit world X position. */
BANKREF_EXTERN(player_get_world_x16)
uint16_t player_get_world_x16(void) BANKED;

/* Trigger the player death animation. */
BANKREF_EXTERN(player_die)
void player_die(void) BANKED;

/* Return 1 if the player is currently in the death state. */
BANKREF_EXTERN(player_is_dying)
uint8_t player_is_dying(void) BANKED;

#endif
