#include "precomp.h"
#include "system.h"

template <typename T>
void vector_swap_and_delete(vector<T>* vec, const uint i)
{
    T last = vec->back();
    T to_delete = vec[i];
    vec->pop_back();
    if (last != to_delete) vec[i] = last;

    // TODO I don't think this delete is necessary anymore, but I'm focussing on other things first
    delete to_delete;
}

Templ8::ParticlesSystem::ParticlesSystem()
{
}

void Templ8::ParticlesSystem::SpawnParticleExplosion(const VisualComponent& tank_visual, const SpatialComponent tank_spatial)
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

Templ8::TanksSystem::TanksSystem(const ParticlesSystem& particle_system)
{
    this->particle_system = particle_system;

    // create the static array of directions if it doesn't exist yet
    if (directions == nullptr)
    {
        directions = new float2[256];
        for (int i = 0; i < 256; i++) directions[i] = make_float2(sinf(i * PI / 128), -cosf(i * PI / 128));
    }
}

void Templ8::TanksSystem::NewTank(Sprite* s, const int2 p, const int2 t, const uint f, const int a)
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
}

void Templ8::TanksSystem::Draw()
{
}

void Templ8::TanksSystem::SpawnParticle(const uint i)
{
    this->particle_system.SpawnParticleExplosion(visuals[i], spatials[i]);
}


void Templ8::TanksSystem::DespawnTank(const uint i)
{
    vector_swap_and_delete(&visuals, i);
    vector_swap_and_delete(&spatials, i);
    vector_swap_and_delete(&targets, i);
    vector_swap_and_delete(&attacks, i);
    vector_swap_and_delete(&collisions, i);
}
