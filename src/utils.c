#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "utils.h"

void draw_text(uint8_t x, uint8_t y, const char* str, uint8_t tile_offset) {
    uint8_t i = 0;
    /* First pass (VBK=0): write tile indices */
    VBK_REG = 0;
    while (str[i]) {
        set_bkg_tile_xy((uint8_t)(x + i), y,
                        (uint8_t)((str[i] - 32U) + tile_offset));
        i++;
    }
    /* Second pass (VBK=1): write palette attributes for the same span */
    VBK_REG = 1;
    for (uint8_t j = 0; j < i; j++) {
        /* GBC background palette slot 2 = font palette (black text) */
        set_bkg_tile_xy((uint8_t)(x + j), y, 0x02U);
    }
    VBK_REG = 0;
}
