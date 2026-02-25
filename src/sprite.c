#include <stdint.h>
#include "sprite.h"

uint8_t sprites_collide(const Sprite *a, const Sprite *b)
{
    if (!a->active || !b->active) return 0;

    /* X-axis overlap (world coordinates) */
    if ((uint8_t)(a->world_x + a->width)  <= b->world_x) return 0;
    if ((uint8_t)(b->world_x + b->width)  <= a->world_x) return 0;

    /* Y-axis overlap: screen_top = hw_y - 16, screen_bot = hw_y - 16 + height
     * Simplifies to: a->hw_y < b->hw_y + b->height  AND
     *                b->hw_y < a->hw_y + a->height           */
    if (a->hw_y >= (uint8_t)(b->hw_y + b->height)) return 0;
    if (b->hw_y >= (uint8_t)(a->hw_y + a->height)) return 0;

    return 1;
}
