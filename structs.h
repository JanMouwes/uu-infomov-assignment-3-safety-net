#pragma once
#include "surface.h"

namespace Tmpl8
{
    struct PhysicalComponent 
    {
        float2 pos;
        float2 direction;
        float2 *directions;
    };

    struct VisualComponent
    {
        SpriteInstance sprite;
        int frame;
    };


    struct Drawable
    {
        float2 pos;
        float2 lastPos;

        /**
         * The base image
         */
        Sprite* sprite;

        /**
         * TODO unknown
         */
        uint* backup = 0;

        Surface* lastTarget = 0;
    };

    inline Drawable init_drawable(const float2 pos, Sprite* sprite)
    {
        return {pos, 0, sprite, nullptr, nullptr};
    }

    void draw_drawable(Surface* target, Drawable drawable, int frame);
}
