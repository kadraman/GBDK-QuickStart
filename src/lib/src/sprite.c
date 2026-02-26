#include <stdint.h>
#include "sprite.h"

uint8_t sprites_collide(const Sprite *a, const Sprite *b)
{
    uint8_t ax, ay, aw, ah;
    uint8_t bx, by, bw, bh;

    if (!a->active || !b->active) return 0;

    /* Resolve hitbox for a */
    ax = (uint8_t)(a->world_x + a->hitbox_x);
    ay = (uint8_t)(a->world_y + a->hitbox_y);
    aw = a->hitbox_w ? a->hitbox_w : a->width;
    ah = a->hitbox_h ? a->hitbox_h : a->height;

    /* Resolve hitbox for b */
    bx = (uint8_t)(b->world_x + b->hitbox_x);
    by = (uint8_t)(b->world_y + b->hitbox_y);
    bw = b->hitbox_w ? b->hitbox_w : b->width;
    bh = b->hitbox_h ? b->hitbox_h : b->height;

    /* AABB overlap test */
    if ((uint8_t)(ax + aw) <= bx) return 0;
    if ((uint8_t)(bx + bw) <= ax) return 0;
    if ((uint8_t)(ay + ah) <= by) return 0;
    if ((uint8_t)(by + bh) <= ay) return 0;

    return 1;
}
