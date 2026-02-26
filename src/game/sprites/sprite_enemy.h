#ifndef SPRITE_ENEMY_H
#define SPRITE_ENEMY_H

#include <stdint.h>
#include "sprite.h"

/* -----------------------------------------------------------------------
 * Enemy sprite – provides init, update, and cleanup for the patrol enemy.
 *
 * The enemy patrols between ENEMY_PATROL_LEFT and ENEMY_PATROL_RIGHT,
 * flipping direction (and sprite) at each boundary.
 * ----------------------------------------------------------------------- */

/* Initialise and allocate the enemy sprite.
 * start_x   : starting world-X position
 * ground_y  : world-Y when standing on the ground
 * tile_base : first VRAM tile slot used by enemy tile data */
void enemy_init(uint8_t start_x, uint8_t ground_y, uint8_t tile_base);

/* Update enemy for one frame.
 * camera_x : current camera X scroll value (for hardware sprite positioning) */
void enemy_update(uint8_t camera_x);

/* Free the enemy sprite and hide its OBJ slot. */
void enemy_cleanup(void);

/* Return a pointer to the enemy's Sprite struct (for collision checks). */
Sprite* enemy_get_sprite(void);

#endif
