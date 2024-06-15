#include "precomp.h"

#define USE_SCALED_PIXELS 0

uint ReadBilerp(Surface& bitmap, float u, float v)
{
    // read from a bitmap with bilinear interpolation.
    // warning: not optimized.
    int iu1 = (int)u % bitmap.width, iv1 = (int)v % bitmap.height;
    int iu2 = (iu1 + 1) % bitmap.width, iv2 = (iv1 + 1) % bitmap.height;
    int fracu = (int)((u - floorf(u)) * 16383), fracv = (int)((v - floorf(v)) * 16383);
    return ScaleColor(bitmap.pixels[iu1 + iv1 * bitmap.width], ((16383 - fracu) * (16384 - fracv)) >> 20) +
        ScaleColor(bitmap.pixels[iu2 + iv1 * bitmap.width], (fracu * (16384 - fracv)) >> 20) +
        ScaleColor(bitmap.pixels[iu1 + iv2 * bitmap.width], ((16383 - fracu) * fracv) >> 20) +
        ScaleColor(bitmap.pixels[iu2 + iv2 * bitmap.width], (fracu * fracv) >> 20);
}

Sprite::Sprite(const char* fileName)
{
    // load original bitmap
    Surface original(fileName);
    // copy to internal data
    frameCount = 1;
    frameSize = original.width;
    pixels = new uint[frameSize * frameSize];
    memcpy(pixels, original.pixels, frameSize * frameSize * 4);
    // fix alpha
    for (int i = 0; i < frameSize * frameSize; i++)
    {
        pixels[i] &= 0xffffff;
        // Eliminate MAGENTA, make it transparent (see tanks.png)
        if (pixels[i] == 0xff00ff) pixels[i] = 0;
            // not transparent, so 100% opaque
        else pixels[i] |= 0xff000000;
    }
}

Sprite::Sprite(const char* fileName, int frames)
{
    // load original bitmap
    Surface original(fileName);
    // copy to internal data
    frameCount = frames;
    frameSize = original.width / frameCount;
    pixels = new uint[frameSize * frameSize * frameCount];
    memcpy(pixels, original.pixels, frameSize * frameSize * frameCount * 4);
}

Sprite::Sprite(const char* fileName, int2 topLeft, int2 bottomRight, int size, int frames)
{
    // load original bitmap
    Surface original(fileName);
    // update alpha
    const uint pixelCount = original.width * original.height;
    for (uint i = 0; i < pixelCount; i++)
    {
        original.pixels[i] &= 0xffffff;
        if (original.pixels[i] == 0xff00ff) original.pixels[i] = 0;
        else original.pixels[i] |= 0xff000000;
    }
    // blur alpha for better outlines
    uint *tmp = new uint[pixelCount], w = original.width;
    for (int j = 0; j < 4; j++)
    {
        for (uint i = w + 1; i < pixelCount - w; i++)
        {
            uint a1 = original.pixels[i + 1] >> 24, a2 = original.pixels[i - 1] >> 24;
            uint a3 = original.pixels[i + w] >> 24, a4 = original.pixels[i - w] >> 24;
            uint ac = original.pixels[i] >> 24;
            uint a = (2 * ac + a1 + a2 + a3 + a4) / 6;
            tmp[i] = (original.pixels[i] & 0xffffff) + (a << 24);
        }
        memcpy(original.pixels + w + 1, tmp + w + 1, pixelCount - (2 * w + 1));
    }
    delete tmp;
    // create quad outline tables
    static float *xleft = 0, *xright = 0, *uleft, *vleft, *uright, *vright;
    if (!xleft)
    {
        xleft = new float[size], xright = new float[size];
        uleft = new float[size], uright = new float[size];
        vleft = new float[size], vright = new float[size];
        for (int i = 0; i < size; i++) xleft[i] = (float)size - 1, xright[i] = 0;
    }
    // produce rotated frames
    left = new int*[frames];
    right = new int*[frames];
    pixels = new uint[size * frames * size];
    memset(pixels, 0, size * frames * size * 4);
    float2 p[4] = {make_float2(-1, -1), make_float2(1, -1), make_float2(1, 1), make_float2(-1, 1)};
    float2 uv[4] = {
        make_float2((float)topLeft.x, (float)topLeft.y), make_float2((float)bottomRight.x, (float)topLeft.y),
        make_float2((float)bottomRight.x, (float)bottomRight.y), make_float2((float)topLeft.x, (float)bottomRight.y)
    };
    for (int miny = size, maxy = 0, frame = 0; frame < frames; frame++)
    {
        // rotate a square
        float2 pos[4];
        float angle = (2 * PI * frame) / frames;
        for (int j = 0; j < 4; j++)
            pos[j].x = (p[j].x * cosf(angle) + p[j].y * sinf(angle)) * 0.35f * size + size / 2,
                pos[j].y = (p[j].x * sinf(angle) - p[j].y * cosf(angle)) * 0.35f * size + size / 2;
        // populate outline tables
        for (int j = 0; j < 4; j++)
        {
            int h, vert0 = j, vert1 = (j + 1) & 3;
            if (pos[vert0].y > pos[vert1].y) h = vert0, vert0 = vert1, vert1 = h;
            const float y0 = pos[vert0].y, y1 = pos[vert1].y, rydiff = 1.0f / (y1 - y0);
            if (y0 == y1) continue;
            const int iy0 = max(1, (int)y0 + 1), iy1 = min(size - 1, (int)y1);
            float x0 = pos[vert0].x, dx = (pos[vert1].x - x0) * rydiff,
                  u0 = uv[vert0].x, du = (uv[vert1].x - u0) * rydiff,
                  v0 = uv[vert0].y, dv = (uv[vert1].y - v0) * rydiff;
            const float f = (float)iy0 - y0;
            x0 += dx * f, u0 += du * f, v0 += dv * f;
            for (int y = iy0; y <= iy1; y++, x0 += dx, u0 += du, v0 += dv)
            {
                if (x0 < xleft[y]) xleft[y] = x0, uleft[y] = u0, vleft[y] = v0;
                if (x0 > xright[y]) xright[y] = x0, uright[y] = u0, vright[y] = v0;
            }
            miny = min(miny, iy0), maxy = max(maxy, iy1);
        }
        // fill the rotated quad using the outline tables
        for (int y = miny; y <= maxy; xleft[y] = (float)size - 1, xright[y++] = 0)
        {
            float x0 = xleft[y], x1 = xright[y], rxdiff = 1.0f / (x1 - x0),
                  u0 = uleft[y], du = (uright[y] - u0) * rxdiff,
                  v0 = vleft[y], dv = (vright[y] - v0) * rxdiff;
            const int ix0 = (int)x0 + 1, ix1 = min(SCRWIDTH - 2, (int)x1);
            u0 += ((float)ix0 - x0) * du, v0 += ((float)ix0 - x0) * dv;
            uint* dest = pixels + frame * size + y * size * frames;
            for (int x = ix0; x <= ix1; x++, u0 += du, v0 += dv) dest[x] = ReadBilerp(original, u0, v0);
        }
    }
    frameCount = frames;
    frameSize = size;
    // calculate 256 versions of the original bitmap, to speedup color blending
    scaledPixels = new uint*[256];
    for (int i = 0; i < 256; i++)
    {
        scaledPixels[i] = (uint*)_aligned_malloc(4 * size * frames * size, 64);
        for (int p = 0; p < size * frames * size; p++)
            scaledPixels[i][p] = ScaleColor(pixels[p], i);
    }
}

void Sprite::ScaleAlpha(uint scale)
{
    for (int i = 0; i < frameSize * frameSize * frameCount; i++)
    {
        int a = ((pixels[i] >> 24) * scale) >> 8;
        pixels[i] = (pixels[i] & 0xffffff) + (a << 24);
    }
}

void SpriteInstance::Draw(Surface* target, float2 pos, int frame)
{
    int frameSize = sprite->frameSize;
    assert (1024 < frameSize);
    uint p0s[1024];
    uint p1s[1024];
    uint p2s[1024];
    uint p3s[1024];
    uint pixs[1024];
    // save the area of target that we are about to overwrite
    if (!backup) backup = new uint[sqr(sprite->frameSize + 1)];
    const int2 intPos = make_int2(pos);
    int x1 = intPos.x - sprite->frameSize / 2, x2 = x1 + sprite->frameSize;
    int y1 = intPos.y - sprite->frameSize / 2, y2 = y1 + sprite->frameSize;
    if (x1 < 0 || y1 < 0 || x2 >= target->width || y2 >= target->height)
    {
        // out of range; skip
        lastTarget = 0;
        return;
    }
    for (int v = 0; v < sprite->frameSize; v++)
        memcpy(backup + v * sprite->frameSize,
               target->pixels + x1 + (y1 + v) * target->width,
               sprite->frameSize * 4);
    lastPos = make_int2(x1, y1);
    lastTarget = target;
    // calculate bilinear weights - these are constant in this case.
    const uint frac_x = (int)(255.0f * (pos.x - floorf(pos.x)));
    const uint frac_y = (int)(255.0f * (pos.y - floorf(pos.y)));
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
#if USE_SCALED_PIXELS
        if (sprite->scaledPixels)
        {
            const uint row_origin = frame * sprite->frameSize + v * stride;

            // get lookup tables
            const uint* src0 = sprite->scaledPixels[interpol_weight_0] + row_origin;
            const uint* src1 = sprite->scaledPixels[interpol_weight_1] + row_origin;
            const uint* src2 = sprite->scaledPixels[interpol_weight_2] + row_origin;
            const uint* src3 = sprite->scaledPixels[interpol_weight_3] + row_origin;

            // left-to-right
            for (int u = 0; u < sprite->frameSize - 1; u++)
            {
                const uint p0 = src0[u];
                const uint p1 = src1[u + 1];
                const uint p2 = src2[u + stride];
                const uint p3 = src3[u + stride + 1];
                const uint pix_colour_argb = p0 + p1 + p2 + p3;
                const uint alpha = pix_colour_argb >> 24;
                dst[u] = ScaleColor(pix_colour_argb, alpha) + ScaleColor(dst[u], 255 - alpha);
            }
        }
        else
#endif
        {
            // fallback
            uint* src = sprite->pixels + frame * sprite->frameSize + v * stride;
            for (int u = 0; u < frameSize - 1; u++, src++)
            {
                p0s[u] = ScaleColor(src[0], interpol_weight_0);
            }

            src = sprite->pixels + frame * sprite->frameSize + v * stride;
            for (int u = 0; u < frameSize - 1; u++, src++)
            {
                p1s[u] = ScaleColor(src[1], interpol_weight_1);
            }

            src = sprite->pixels + frame * frameSize + v * stride;
            for (int u = 0; u < sprite->frameSize - 1; u++, src++)
            {
                p2s[u] = ScaleColor(src[stride], interpol_weight_2);
            }

            src = sprite->pixels + frame * frameSize + v * stride;
            for (int u = 0; u < sprite->frameSize - 1; u++, src++)
            {
                p3s[u] = ScaleColor(src[stride + 1], interpol_weight_3);
            }

            for (int u = 0; u < frameSize - 1; u++)
            {
                pixs[u] = p0s[u] + p1s[u] + p2s[u] + p3s[u];
            }
            
            for (int u = 0; u < frameSize - 1; u++, dst++)
            {
                uint alpha = pixs[u] >> 24;
                if (alpha) *dst = ScaleColor(pixs[u], alpha) + ScaleColor(*dst, 255 - alpha);
            }
        }
    }
}

void SpriteInstance::DrawAdditive(Surface* target, float2 pos, int frame)
{
    // save the area of target that we are about to overwrite
    if (!backup) backup = (uint*)_aligned_malloc(4 * sprite->frameSize * sprite->frameSize, 64);
    int2 intPos = make_int2(pos);
    int x1 = intPos.x - sprite->frameSize / 2, x2 = x1 + sprite->frameSize;
    int y1 = intPos.y - sprite->frameSize / 2, y2 = y1 + sprite->frameSize;
    if (x1 < 0 || y1 < 0 || x2 >= target->width || y2 >= target->height)
    {
        // out of range; skip
        lastTarget = 0;
        return;
    }
    for (int v = 0; v < sprite->frameSize; v++)
        memcpy(backup + v * sprite->frameSize,
               target->pixels + x1 + (y1 + v) * target->width,
               sprite->frameSize * 4);
    // draw the sprite frame
    for (int v = 0; v < sprite->frameSize; v++)
        for (int u = 0; u < sprite->frameSize; u++)
        {
            uint* dst = target->pixels + x1 + u + (y1 + v) * target->width;
            uint pix = sprite->pixels[frame * sprite->frameSize + u + v * sprite->frameCount * sprite->frameSize];
            *dst = AddBlend(*dst, pix);
        }
    // remember where we drew so it can be removed later
    lastPos = make_int2(x1, y1);
    lastTarget = target;
}

void SpriteInstance::Remove()
{
    // use the stored pixels to restore the rectangle affected by the sprite.
    // note: sprites must be removed in reverse order to guarantee correct removal.
    if (lastTarget)
        for (int v = 0; v < sprite->frameSize; v++)
        {
            memcpy(lastTarget->pixels + lastPos.x + (lastPos.y + v) * lastTarget->width,
                   backup + v * sprite->frameSize, sprite->frameSize * 4);
        }
}
