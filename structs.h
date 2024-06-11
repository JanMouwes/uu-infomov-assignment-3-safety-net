#pragma once
#include "surface.h"

namespace Tmpl8
{
    struct PhysicalComponent 
    {
        float2 pos;
        float2 target;
        float2 dir;
        // TODO: make static inline / a proper global LUT
        float2 *directions;
    };

    struct VisualComponent
    {
        SpriteInstance sprite;
        int frame;
    };
}
