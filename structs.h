#pragma once

namespace Tmpl8
{
    struct SpatialComponent 
    {
        float2 pos;
        float2 dir;
    };

    struct MovementComponent
    {
        float2 velocity;
    };

    struct SteerComponent
    {
        float steer;
    };

    struct TargetComponent
    {
        float2 target;
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
