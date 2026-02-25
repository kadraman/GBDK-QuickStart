#ifndef UTILS_H
#define UTILS_H

#include <gb/cgb.h>
#include <stdint.h>

/* Draw a null-terminated ASCII string as background tiles at (x, y).
   tile_offset is the VRAM tile index corresponding to ASCII 32 (space).
   Text tiles are assigned GBC palette 2 (font palette: black text). */
void draw_text(uint8_t x, uint8_t y, const char* str, uint8_t tile_offset);

#endif
