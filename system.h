namespace Templ8
{
    /*
     * TargetsAttractSystem loops over each target component updating the steer component towards the target. It assumes
     * that each entity registered in targets is also registered in spatials and steers. It does not assume that each
     * entity registered in spatials and steers are also registered with targets.
     */
    void TargetsAttractSystem(vector<TargetComponent> targets, vector<SpatialComponent> spatials,
                              vector<SteerComponent> steers)
    {
        for (int i = 0; i < targets.size(); i++)
        {
            TargetComponent t = targets.at(i);
            SpatialComponent s = spatials.at(i);

            float2 toTarget = normalize(t.target - s.pos);
            float2 toRight = make_float2(-s.dir.y, s.dir.x);

            steers[i].steer = 2 * dot(toRight, toTarget);
        }
    }

    void MountainsRepelSystem(vector<SpatialComponent> spatials, vector<SteerComponent> steers)
    {
    }


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
        TanksSystem(ParticlesSystem* particles_system, BulletsSystem* bullets_system, vector<float3>* peaks);
        void SpawnTank(Sprite* s, int2 p, int2 t, uint f, int a);

        void Tick();

        void Draw();
        Tank GetTank(uint i) const;

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
        vector<SteerComponent> steers;

        ParticlesSystem* particles_system;
        BulletsSystem* bullets_system;
        vector<float3>* peaks;

        /**
         * Prevents simultaneous firing
         */
        uint cooldown = 0;
    };
}
