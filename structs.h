#pragma once

namespace Tmpl8
{
    struct SpatialComponent
    {
        float2 pos;
        float2 dir;
        float2 velocity = make_float2(0);
    };

    struct SteerComponent
    {
        float2 target;
        float steer;
    };


    struct VisualComponent
    {
        SpriteInstance sprite;
        uint frame;
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
