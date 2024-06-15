#pragma once

// Setting this too high causes a stack-overflow in Game::DrawSprite
#define SPRITE_SOA_SIZE 464

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

        float2 tank1_poss[SPRITE_SOA_SIZE];
        uint* tank1_backups[SPRITE_SOA_SIZE];
        Surface* tank1_last_targets[SPRITE_SOA_SIZE];
        int2 tank1_last_poss[SPRITE_SOA_SIZE];
        void DrawSprite(Sprite s, float2 poss[SPRITE_SOA_SIZE], Surface* target, uint* backups[SPRITE_SOA_SIZE], Surface* last_targets[SPRITE_SOA_SIZE], int2 last_poss[SPRITE_SOA_SIZE]);
        void RemoveSprite(Sprite s, uint* backups[SPRITE_SOA_SIZE], Surface* last_targets[SPRITE_SOA_SIZE], int2 last_poss[SPRITE_SOA_SIZE]);
        
        // static data, for global access
        static inline Map map; // the map
        static inline vector<Actor*> actorPool; // actor pool
        static inline vector<float3> peaks; // mountain peaks to evade
        static inline vector<Particle*> sand; // sand particles
        static inline Grid grid; // actor grid for faster range queries
        static inline int coolDown = 0; // used to prevent simultaneous firing
    };
} // namespace Tmpl8
