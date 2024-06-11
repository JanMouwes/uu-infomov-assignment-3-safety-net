#include "precomp.h"
#include "structs.h"

void Tmpl8::draw_drawable(Surface* target, Drawable drawable, const int frame)
{
    const Sprite* sprite = drawable.sprite;
    // save the area of target that we are about to overwrite
    if (!drawable.backup) drawable.backup = new uint[sqr(sprite->frameSize + 1)];
    const int2 intPos = make_int2(drawable.pos);
    int x1 = intPos.x - sprite->frameSize / 2, x2 = x1 + sprite->frameSize;
    int y1 = intPos.y - sprite->frameSize / 2, y2 = y1 + sprite->frameSize;
    if (x1 < 0 || y1 < 0 || x2 >= target->width || y2 >= target->height)
    {
        // out of range; skip
        drawable.lastTarget = 0;
        return;
    }
    for (int v = 0; v < sprite->frameSize; v++)
        memcpy(drawable.backup + v * sprite->frameSize,
               target->pixels + x1 + (y1 + v) * target->width,
               sprite->frameSize * 4);
    drawable.lastPos = make_int2(x1, y1);
    drawable.lastTarget = target;
    // calculate bilinear weights - these are constant in this case.
    const uint frac_x = (int)(255.0f * (drawable.pos.x - floorf(drawable.pos.x)));
    const uint frac_y = (int)(255.0f * (drawable.pos.y - floorf(drawable.pos.y)));
    const uint interpol_weight_0 = (frac_x * frac_y) >> 8;
    const uint interpol_weight_1 = ((255 - frac_x) * frac_y) >> 8;
    const uint interpol_weight_2 = (frac_x * (255 - frac_y)) >> 8;
    const uint interpol_weight_3 = ((255 - frac_x) * (255 - frac_y)) >> 8;
    // draw the sprite frame
    const uint stride = sprite->frameCount * sprite->frameSize;

    // top-to-bottom
    for (int v = 0; v < sprite->frameSize - 1; v++)
    {
        uint* dst = target->pixels + x1 + (y1 + v) * target->width;
        {
            // fallback
            uint* src = sprite->pixels + frame * sprite->frameSize + v * stride;
            for (int u = 0; u < sprite->frameSize - 1; u++, src++, dst++)
            {
                const uint p0 = ScaleColor(src[0], interpol_weight_0);
                const uint p1 = ScaleColor(src[1], interpol_weight_1);
                const uint p2 = ScaleColor(src[stride], interpol_weight_2);
                const uint p3 = ScaleColor(src[stride + 1], interpol_weight_3);
                const uint pix = p0 + p1 + p2 + p3;
                const uint alpha = pix >> 24;
                if (alpha) *dst = ScaleColor(pix, alpha) + ScaleColor(*dst, 255 - alpha);
            }
        }
    }
}
