#pragma once

namespace Templ8
{
    class ParticlesSystem
    {
    public:
        ParticlesSystem();
        void SpawnParticleExplosion(const VisualComponent& tank_visual, SpatialComponent tank_spatial);

    private:
        vector<vector<uint>> colours;
        vector<vector<SpatialComponent>> spatials;
    };

    class BulletsSystem
    {
    public:
        BulletsSystem();
        void SpawnBullet(SpatialComponent spatial, int army, uint frame);

    private:
        static inline Sprite* flash = nullptr;
        static inline Sprite* bullet = nullptr;
        static inline float2* directions;

        vector<VisualComponent> visuals;
        vector<SpatialComponent> spatials;
        vector<AttackComponent> attacks;
        vector<LifetimeComponent> lifetimes;
        vector<SpriteInstance> flashSprites;
    };

    class TanksSystem
    {
    public:
        TanksSystem(ParticlesSystem* particle_system, BulletsSystem* bullets_system, Grid* grid);
        Tank* SpawnTank(Sprite* s, int2 p, int2 t, uint f, int a);

        void Tick();

        void Draw();

    private:
        static inline float2* directions;

        void SpawnParticle(uint i);
        void SpawnBullet(uint i);
        void DespawnTank(uint i);

        vector<VisualComponent> visuals;
        vector<SpatialComponent> spatials;
        vector<TargetComponent> targets;
        vector<AttackComponent> attacks;
        vector<CollisionComponent> collisions;

        ParticlesSystem* particles_system;
        BulletsSystem* bullets_system;
        Grid* grid;

        /**
         * Prevents simultaneous firing
         */
        uint cooldown = 0;
    };
}
