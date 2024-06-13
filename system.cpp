#include "precomp.h"
#include "system.h"

template <typename T>
void vector_swap_and_delete(vector<T>* vec, const uint i)
{
    T last = vec->back();
    T to_delete = (*vec)[i];
    vec->pop_back();

    bool was_last_i = i == vec->size();
    if (!was_last_i) (*vec)[i] = last;
}

Templ8::ParticlesSystem::ParticlesSystem()
{
}

void Templ8::ParticlesSystem::SpawnParticleExplosion(const VisualComponent& tank_visual,
                                                     const SpatialComponent tank_spatial)
{
    // read the pixels from the sprite of the specified tank
    const Sprite* sprite = tank_visual.sprite.sprite;
    const uint size = sprite->frameSize;
    const uint stride = sprite->frameSize * sprite->frameCount;
    const uint* src = sprite->pixels + tank_visual.frame * size;

    vector<uint> colour;
    vector<SpatialComponent> spatial;
    vector<float2> dir;
    for (uint y = 0; y < size; y++)
        for (uint x = 0; x < size; x++)
        {
            const uint pixel = src[x + y * stride];
            const uint alpha = pixel >> 24;
            if (alpha > 64)
                for (int i = 0; i < 2; i++) // twice for a denser cloud
                {
                    colour.push_back(pixel & 0xffffff);
                    const float fx = tank_spatial.pos.x - size * 0.5f + x;
                    const float fy = tank_spatial.pos.y - size * 0.5f + y;
                    spatial.push_back({ make_float2(fx, fy), make_float2(0, 0) });
                }
        }

    colours.push_back(colour);
    spatials.push_back(spatial);
}


Templ8::BulletsSystem::BulletsSystem()
{
    // TODO un-duplicate this
    // create the static array of directions if it doesn't exist yet
    if (directions == nullptr)
    {
        directions = new float2[256];
        for (int i = 0; i < 256; i++) directions[i] = make_float2(sinf(i * PI / 128), -cosf(i * PI / 128));
    }

    if (flash == nullptr)
    {
        // load static sprite data if not done yet
        flash = new Sprite("assets/flash.png");
        bullet = new Sprite("assets/bullet.png", make_int2(2, 2), make_int2(31, 31), 32, 256);
    }
}

void Templ8::BulletsSystem::SpawnBullet(const SpatialComponent spatial, const int army, const uint frame)
{
    // create sprite instances based on the static sprite data
    // set position and direction
    // This assumes that a tank is constructed before any bullet is constructed.
    // for keeping track of bullet lifetime

    visuals.push_back({ SpriteInstance(bullet), frame });
    spatials.push_back({ make_int2(spatial.pos + 20 * spatial.dir), directions[frame] });
    lifetimes.push_back({ 0 });
    attacks.push_back({ army, 0 });
    flashSprites.push_back(SpriteInstance(flash));
}


Templ8::TanksSystem::TanksSystem(ParticlesSystem* particle_system, BulletsSystem* bullets_system, Grid* grid)
{
    this->particles_system = particle_system;
    this->bullets_system = bullets_system;
    this->grid = grid;

    // create the static array of directions if it doesn't exist yet
    if (directions == nullptr)
    {
        directions = new float2[256];
        for (int i = 0; i < 256; i++) directions[i] = make_float2(sinf(i * PI / 128), -cosf(i * PI / 128));
    }
}

Tank* Templ8::TanksSystem::SpawnTank(Sprite* s, const int2 p, const int2 t, const uint f, const int a)
{
    // create sprite instance based on existing sprite
    // set initial orientation / sprite visual.frame; 0: north; 64: east; 128: south; 192: west
    visuals.push_back({ SpriteInstance(s), f });

    // set direction based on specified orientation
    spatials.push_back({ make_float2(p), directions[f] });
    targets.push_back({ make_float2(t) });

    // assign tank to the specified army
    attacks.push_back({ a, 0 });
    collisions.push_back({ false });

    return new Tank(
        visuals.back(),
        spatials.back(),
        targets.back(),
        attacks.back(),
        collisions.back()
    );
}

void Templ8::TanksSystem::Tick()
{
    for (uint i = 0; i < collisions.size(); ++i)
    {
        const CollisionComponent collision = collisions[i];
        if (collision.hit_by_bullet)
        {
            SpawnParticle(i);
            DespawnTank(i);
            i--;
        }
    }

    for (uint i = 0; i < attacks.size(); ++i)
    {
        AttackComponent* attack = &attacks[i];
        const SpatialComponent spatial = spatials[i];

        // fire bullet if cooled down and enemy is in range
        if (attack->cool_down > 200 && cooldown > 4)
        {
            // query a grid to rapidly obtain a list of nearby tanks
            ActorList& nearby = grid->FindNearbyTanks(spatial.pos + spatial.dir * 200);
            for (int nearby_i = 0; nearby_i < nearby.count; nearby_i++)
                if (nearby.tank[nearby_i]->attack.army != attack->army)
                {
                    float2 to_actor = normalize(nearby.tank[nearby_i]->spatial.pos - spatial.pos);
                    if (dot(to_actor, spatial.dir) > 0.8f /* within view cone*/)
                    {
                        SpawnBullet(i);
                        attack->cool_down = 0;
                        cooldown = 0;
                        break;
                    }
                }
        }
        attack->cool_down++;
    }
}

void Templ8::TanksSystem::Draw()
{
}

void Templ8::TanksSystem::SpawnParticle(const uint i)
{
    this->particles_system->SpawnParticleExplosion(visuals[i], spatials[i]);
}

void Templ8::TanksSystem::SpawnBullet(const uint i)
{
    const SpatialComponent spatial = spatials[i];
    const AttackComponent attack = attacks[i];
    const VisualComponent visual = visuals[i];

    bullets_system->SpawnBullet(spatial, attack.army, visual.frame);
}


void Templ8::TanksSystem::DespawnTank(const uint i)
{
    vector_swap_and_delete(&visuals, i);
    vector_swap_and_delete(&spatials, i);
    vector_swap_and_delete(&targets, i);
    vector_swap_and_delete(&attacks, i);
    vector_swap_and_delete(&collisions, i);
}
