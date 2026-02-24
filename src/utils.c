#include <gb/gb.h>
#include <stdint.h>
#include "utils.h"

void draw_text(uint8_t x, uint8_t y, const char* str, uint8_t tile_offset) {
    uint8_t i = 0;
    while (str[i]) {
        set_bkg_tile_xy((uint8_t)(x + i), y, (uint8_t)((str[i] - 32U) + tile_offset));
        i++;
    }
}
