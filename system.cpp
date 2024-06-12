#include "precomp.h"
#include "system.h"

#include "game.h"

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


Templ8::TanksSystem::TanksSystem(ParticlesSystem* particles_system, BulletsSystem* bullets_system,
                                 vector<float3>* peaks): particles_system(particles_system),
                                                         bullets_system(bullets_system),
                                                         peaks(peaks)
{
    // create the static array of directions if it doesn't exist yet
    if (directions == nullptr)
    {
        directions = new float2[256];
        for (int i = 0; i < 256; i++) directions[i] = make_float2(sinf(i * PI / 128), -cosf(i * PI / 128));
    }
}

void Templ8::TanksSystem::SpawnTank(Sprite* s, const int2 p, const int2 t, const uint f, const int a)
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
    steers.push_back({ 0 });
}

void Templ8::TanksSystem::Tick()
{
    Game::grid.Populate(spatials);
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
            ActorList& nearby = Game::grid.FindNearbyTanks(spatial.pos + spatial.dir * 200, i);
            for (int nearby_i = 0; nearby_i < nearby.count; nearby_i++)
            {
                AttackComponent other_attack = attacks[nearby.tank[nearby_i]];
                SpatialComponent other_spatial = spatials[nearby.tank[nearby_i]];
                if (other_attack.army != attack->army)
                {
                    float2 to_actor = normalize(other_spatial.pos - spatial.pos);
                    if (dot(to_actor, spatial.dir) > 0.8f /* within view cone*/)
                    {
                        SpawnBullet(i);
                        attack->cool_down = 0;
                        cooldown = 0;
                        break;
                    }
                }
            }
        }
        attack->cool_down++;
    }

    for (uint i = 0; i < spatials.size(); ++i)
    {
        TargetComponent target = targets[i];
        SpatialComponent spatial = spatials[i];
        SteerComponent steer = steers[i];

        // accumulate forces for steering left or right
        // 1. target attracts
        float2 toTarget = normalize(target.target - spatial.pos);
        float2 toRight = make_float2(-spatial.dir.y, spatial.dir.x);
        steer.steer = 2 * dot(toRight, toTarget);
        // 2. mountains repel
        float2 probePos = spatial.pos + 8 * spatial.dir;
        for (uint s = peaks->size(), peak_i = 0; peak_i < s; peak_i++)
        {
            float3 peak = (*peaks)[peak_i];

            float peakMag = peak.z / 2;
            float2 toPeak = make_float2(peak.x, peak.y) - probePos;
            float sqrDist = dot(toPeak, toPeak);
            if (sqrDist < sqrf(peakMag))
                toPeak = normalize(toPeak),
                    steer.steer -= dot(toRight, toPeak) * peakMag / sqrtf(sqrDist);
        }
        // 3. evade other tanks
        ActorList& nearby = Game::grid.FindNearbyTanks(spatial.pos, i);
        for (int nearby_i = 0; nearby_i < nearby.count; nearby_i++)
        {
            uint tank_id = nearby.tank[nearby_i];
            float2 toActor = spatials[tank_id].pos - spatial.pos;
            float sqrDist = dot(toActor, toActor);
            if (sqrDist < 400 && dot(toActor, spatial.dir) > 0.35f)
            {
                steer.steer -= (400 - sqrDist) * 0.02f * dot(toActor, toRight) > 0 ? 1 : -1;
                break;
            }
        }

        VisualComponent visual = visuals[i];
        // adjust heading and move
        spatial.velocity = make_float2(0.5f);
        if (steer.steer < -0.2f)
            visual.frame = (visual.frame + 255 /* i.e. -1 */) & 255, spatial.dir = directions[
                visual.frame], spatial.velocity = make_float2(0.35f);
        else if (steer.steer > 0.2f)
            visual.frame = (visual.frame + 1) & 255, spatial.dir = directions[visual.frame],
                make_float2(0.35f);
        else
        {
            // draw tank tracks, only when not turning
            float2 perp(-spatial.dir.y, spatial.dir.x);
            float2 trackPos1 = spatial.pos - 9 * spatial.dir + 4.5f * perp;
            float2 trackPos2 = spatial.pos - 9 * spatial.dir - 5.5f * perp;
            Map::bitmap->BlendBilerp(trackPos1.x, trackPos1.y, 0, 12);
            Map::bitmap->BlendBilerp(trackPos2.x, trackPos2.y, 0, 12);
        }
        spatial.pos += spatial.dir * spatial.velocity;
    }
}

void Templ8::TanksSystem::Draw()
{
}

Tank Templ8::TanksSystem::GetTank(const uint i) const
{
    return Tank(
        &spatials[i],
        &visuals[i],
        &targets[i],
        &attacks[i]
    );
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
    vector_swap_and_delete(&steers, i);
}
