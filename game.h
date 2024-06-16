#pragma once

// Sandstorm related constants
#define MAX_SAND 7500

// Tank related constants
#define MAX_ARMY_SIZE 464
#define TANK_SPRITE_FRAME_SIZE 36
#define TANK_SPRITE_FRAMES 256

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
            if (key == GLFW_KEY_C)
            {
                int2 cursorPos = map.ScreenToMap(mousePos);
                printf("cursorPos: (%d, %d)\n", cursorPos.x, cursorPos.y);
            }
            
            if (key == GLFW_KEY_F)
            {
                debug_print_frame_time = !debug_print_frame_time;
            }

            if (key == GLFW_KEY_P)
            {
                is_tick_paused = !is_tick_paused;
            }
        }

        // data members
        float zoom = 100; // map zoom
        int2 mousePos, dragStart, focusStart; // mouse / map interaction
        bool mouseDown = false; // keeping track of mouse button status
        Sprite *tank1, *tank2; // tank sprites
        Sprite* bush[3]; // bush sprite
        SpriteInstance* pointer; // mouse pointer sprite

        uint next_tank1;
        float2 tank1_poss[MAX_ARMY_SIZE];
        int2 tank1_int_poss[MAX_ARMY_SIZE];
        int tank1_x1s[MAX_ARMY_SIZE], tank1_x2s[MAX_ARMY_SIZE], tank1_y1s[MAX_ARMY_SIZE], tank1_y2s[MAX_ARMY_SIZE];
        uint tank1_frac_xs[MAX_ARMY_SIZE], tank1_frac_ys[MAX_ARMY_SIZE];
        uint tank1_interpol_weight_0s[MAX_ARMY_SIZE],  tank1_interpol_weight_1s[MAX_ARMY_SIZE], tank1_interpol_weight_2s[MAX_ARMY_SIZE], tank1_interpol_weight_3s[MAX_ARMY_SIZE];
        uint tank1_p0ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank1_p1ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank1_p2ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank1_p3ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank1_pixss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        int tank1_frames[MAX_ARMY_SIZE];
        uint* tank1_backups[MAX_ARMY_SIZE];
        Surface* tank1_last_targets[MAX_ARMY_SIZE];
        int2 tank1_last_poss[MAX_ARMY_SIZE];

        uint next_tank2;
        // Updated by Tank::Tick()
        float2 tank2_poss[MAX_ARMY_SIZE];
        int tank2_frames[MAX_ARMY_SIZE];
        // Not updated by Tank::Tick()
        int2 tank2_int_poss[MAX_ARMY_SIZE];
        int tank2_x1s[MAX_ARMY_SIZE], tank2_x2s[MAX_ARMY_SIZE], tank2_y1s[MAX_ARMY_SIZE], tank2_y2s[MAX_ARMY_SIZE];
        uint tank2_frac_xs[MAX_ARMY_SIZE], tank2_frac_ys[MAX_ARMY_SIZE];
        uint tank2_interpol_weight_0s[MAX_ARMY_SIZE],  tank2_interpol_weight_1s[MAX_ARMY_SIZE], tank2_interpol_weight_2s[MAX_ARMY_SIZE], tank2_interpol_weight_3s[MAX_ARMY_SIZE];
        uint tank2_p0ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank2_p1ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank2_p2ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank2_p3ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint tank2_pixss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1];
        uint* tank2_backups[MAX_ARMY_SIZE];
        Surface* tank2_last_targets[MAX_ARMY_SIZE];
        int2 tank2_last_poss[MAX_ARMY_SIZE];

        /*
         * DrawSprite expects that each pointer argument is an  array of size 'total'. Except for:
         * - target is a pointer to the surface to draw to
         * - p0ss, p1ss, p2ss, p3ss, and pixss are multidimensional arrays of shape [total][s.frameSize] projected to
         *   the shape [total * s.frameSize]
         */
        void DrawSprite(
            Sprite s,
            float2* poss,
            int* frames,
            int2* int_poss,
            int* x1s, int* x2s, int* y1s, int* y2s,
            uint* frac_xs, uint* frac_ys,
            uint* interpol_weight_0s, uint* interpol_weight_1s, uint* interpol_weight_2s, uint* interpol_weight_3s,
            
            uint p0ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1], uint p1ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1], uint p2ss[MAX_ARMY_SIZE - 1][TANK_SPRITE_FRAME_SIZE - 1], uint p3ss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1], uint pixss[MAX_ARMY_SIZE][TANK_SPRITE_FRAME_SIZE - 1],
            Surface** last_targets,
            int2* last_poss,
            uint** backups,
            Surface* target,
            uint total);
        /*
         * Each pointer argument should be an array of size 'total'.
         */
        void RemoveSprite(Sprite s, uint** backups, Surface** last_targets, int2* last_poss, uint total);
        
        // static data, for global access
        static inline bool debug_print_frame_time;
        static inline bool is_tick_paused;
        static inline Map map; // the map
        static inline vector<Actor*> actorPool; // actor pool
        static inline vector<float3> peaks; // mountain peaks to evade
        static inline vector<Particle*> sand; // sand particles
        static inline Grid grid; // actor grid for faster range queries
        static inline int coolDown = 0; // used to prevent simultaneous firing
    };
} // namespace Tmpl8
