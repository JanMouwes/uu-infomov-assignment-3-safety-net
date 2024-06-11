#pragma once

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

    struct AnimateComponent
    {
        uint frame_change;
    };

    struct AttackComponent
    {
        int army;
        int cool_down;
    };

    struct CollisionComponent
    {
        bool hit_by_bullet;
    };

    struct LifetimeComponent
    {
        /*
         * age is the number of ticks since the entity was spawned.
         */
        int age;
    };
}
