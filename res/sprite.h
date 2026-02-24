#ifndef SPRITE_H
#define SPRITE_H

#include <gb/gb.h>
#include <stdint.h>

#define SPRITE_TILE_COUNT      32U
#define SPRITE_PALETTE_COUNT    1U
#define SPRITE_FRAME_COUNT      8U
#define SPRITE_TILES_PER_FRAME  4U

extern const palette_color_t sprite_palettes[4];
extern const uint8_t sprite_tiles[512];

#endif
