#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "states.h"
#include "../res/background.h"
#include "../res/font.h"
#include "../res/sprite.h"

void main(void) {
    DISPLAY_OFF;

    /* Load background tiles into VRAM bank 0 */
    set_bkg_data(0, BACKGROUND_TILE_COUNT, background_tiles);
    /* Load font tiles into VRAM bank 0 after background tiles */
    set_bkg_data(BACKGROUND_TILE_COUNT, FONT_TILE_COUNT, font_tiles);

    /* Load tiles into VRAM bank 1 (GBC extended bank) for tile attributes */
    VBK_REG = 1;
    set_bkg_data(0, BACKGROUND_TILE_COUNT, background_tiles);
    VBK_REG = 0;

    /* Set GBC background palettes (slots 0-1: sky and ground colors) */
    set_bkg_palette(0, BACKGROUND_PALETTE_COUNT, background_palettes);
    /* Set GBC font palette (slot 2: black text on sky-matching background) */
    set_bkg_palette(2, FONT_PALETTE_COUNT, font_palettes);
    /* Set GBC sprite palettes */
    set_sprite_palette(0, SPRITE_PALETTE_COUNT, sprite_palettes);

    /* Load sprite tiles */
    set_sprite_data(0, SPRITE_TILE_COUNT, sprite_tiles);

    /* Use 8x16 sprite mode for 2-tile-tall sprites */
    SPRITES_8x16;

    DISPLAY_ON;
    SHOW_BKG;
    SHOW_SPRITES;

    /* Start with title screen */
    switch_state(STATE_TITLE_SCREEN);

    /* Main game loop */
    while(1) {
        vsync();
        run_current_state();
    }
}
