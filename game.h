#pragma once
#include "entity.h"

namespace Tmpl8
{
    class Game : public TheApp
    {
    public:
        // game flow methods
        void Init();
        void HandleInput();
        void Tick(float deltaTime);

        void Shutdown()
        {
            /* implement if you want to do something on exit */
        }

        // input handling
        void MouseUp(int button) { mouseDown = false; }
        void MouseDown(int button) { mouseDown = true; }
        void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
        void MouseWheel(float y);

        void KeyUp(int key)
        {
            /* implement if you want to handle keys */
        }

        void KeyDown(int key)
        {
            /* implement if you want to handle keys */
        }

        // data members
        float zoom = 100; // map zoom
        int2 mousePos, dragStart, focusStart; // mouse / map interaction
        bool mouseDown = false; // keeping track of mouse button status
        Sprite *tank1, *tank2; // tank sprites
        Sprite* bush[3]; // bush sprite
        SpriteInstance* pointer; // mouse pointer sprite
        
        // static data, for global access
        static inline Map map; // the map
        static inline vector<Actor*> actorPool; // actor pool
        static inline vector<float3> peaks; // mountain peaks to evade
        static inline vector<Particle*> sand; // sand particles
        static inline Grid grid; // actor grid for faster range queries
        static inline int coolDown = 0; // used to prevent simultaneous firing

        static inline float2* directions;

        static inline Sprite* flash = 0, *bullet = 0;
        
        // Entities
        entity NewTank(Sprite* s, int2 p, int2 t, int f, int a)
        {
            // create the static array of directions if it doesn't exist yet
            if (directions == nullptr)
            {
                directions = new float2[256];
                for (int i = 0; i < 256; i++) directions[i] = make_float2(sinf(i * PI / 128), -cosf(i * PI / 128));
            }
            
            entity e = Templ8::NewEntity();
            visuals[e] = {SpriteInstance(s)};
            physicals[e] = {make_float2(p), make_float2(t), directions[f]};
            attacks[e] = { a, 0};
            collisions[e] = {false};

            return e;
        }

        entity NewBullet(int2 p, int f, int a)
        {
            if (flash == nullptr || bullet == nullptr)
            {
                // load static sprite data if not done yet
                flash = new Sprite("assets/flash.png");
                bullet = new Sprite("assets/bullet.png", make_int2(2, 2), make_int2(31, 31), 32, 256);
            }

            entity e = Templ8::NewEntity();
	        visuals[e] = {SpriteInstance(bullet), f};
	        physicals[e] = {make_float2(p), 0, directions[f]};
            lifetimes[e] = { 0 };
            attacks[e].army = a;
            // TODO: How do we want to deal with a bullet having an "on collision draw a different sprite" behaviour? 
            // My initial thoughts are to treat it as an animation that the collision system can trigger? Likewise the
            // tank particle explosion effect can can be triggered on collision. The problem can also be seen as one
            // entity having multiple sprites, which we can't include in our ECS-system as of right now.
            // flashSprite = SpriteInstance(flash);
            return e;
        }

        entity NewParticle(Sprite* s, int2 p, uint d)
        {
            entity e = Templ8::NewEntity();
            physicals[e].pos =  make_float2(p);
            physicals[e].dir =  make_float2(p);
            visuals[e].sprite = SpriteInstance(s);
            animations[e] = { d };

            return e;
        }
        
        // Components
        static inline vector<PhysicalComponent> physicals;
        static inline vector<VisualComponent> visuals;
        static inline vector<AnimateComponent> animations;
        static inline vector<AttackComponent> attacks;
        static inline vector<CollisionComponent> collisions;
        static inline vector<LifetimeComponent> lifetimes;
    };
} // namespace Tmpl8
