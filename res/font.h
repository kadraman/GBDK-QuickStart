#ifndef FONT_H
#define FONT_H

#include <gb/cgb.h>
#include <stdint.h>

#define FONT_TILE_COUNT    96U
#define FONT_PALETTE_COUNT 1U

extern const palette_color_t font_palettes[4];
extern const uint8_t font_tiles[1536];

#endif
