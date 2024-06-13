#pragma once

namespace Tmpl8
{
    class Sprite
    {
    public:
        Sprite(const char* fileName);
        Sprite(const char* fileName, int2 topLeft, int2 bottomRight, int size, int frames);
        Sprite(const char* fileName, int frames);
        void ScaleAlpha(uint scale);
        /**
         * Pixel ARGB-colour values
         */
        uint* pixels;
        /**
         * 'Scale Lookup Table', goes from scale to 'Pixel Lookup Table'
         * Map<scale, Map<pixel index, scaled colour>>
         */
        uint** scaledPixels = 0;
        /**
         * Number of frames in the animation.
         */
        int frameCount;
        /**
         * Width per frame in the animation, in pixels.
         */
        int frameSize;
        int **left, **right;
    };

    class SpriteInstance
    {
    public:
        SpriteInstance() = default;

        SpriteInstance(Sprite* const s) : sprite(s)
        {
        }

        void Draw(Surface* target, float2 pos, uint frame);
        void DrawAdditive(Surface* target, float2 pos, uint frame);
        void Remove();
        Sprite* sprite = 0;
        uint* backup = 0;
        int2 lastPos;
        Surface* lastTarget = 0;
    };
} // namespace Tmpl8
