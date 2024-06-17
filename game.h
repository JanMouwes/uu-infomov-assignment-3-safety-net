#pragma once

inline uint To1D(uint x, uint y, uint z, uint width, uint height)
{
    return z * width * height + y * width + x;
}

inline uint To1D(uint x, uint y, uint z, uint square)
{
    return To1D(x, y, z, square, square);
}

// Sandstorm related constants
#define MAX_SAND 7500 // are created total
#define THIRD_MAX_SAND 2500 // distributed evenly across 3 different sand sprites
#define BUSH_0_FRAME_SIZE 10
#define BUSH_0_FRAMES 256
#define BUSH_1_FRAME_SIZE 14
#define BUSH_1_FRAMES 256
#define BUSH_2_FRAME_SIZE 20
#define BUSH_2_FRAMES 256

// Tank related constants
#define MAX_ARMY_SIZE 464 // Each army spawns 464 tanks
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

        /*
         * =============================================================================================================
         * SoA set-up. Written in order of initialization in Game::Init()
         * =============================================================================================================
         */
        uint next_tank1;
        float2 tank1_poss[MAX_ARMY_SIZE];
        int2 tank1_int_poss[MAX_ARMY_SIZE];
        int tank1_x1s[MAX_ARMY_SIZE], tank1_x2s[MAX_ARMY_SIZE], tank1_y1s[MAX_ARMY_SIZE], tank1_y2s[MAX_ARMY_SIZE];
        uint tank1_frac_xs[MAX_ARMY_SIZE], tank1_frac_ys[MAX_ARMY_SIZE];
        uint tank1_interpol_weight_0s[MAX_ARMY_SIZE], tank1_interpol_weight_1s[MAX_ARMY_SIZE], tank1_interpol_weight_2s[
                 MAX_ARMY_SIZE], tank1_interpol_weight_3s[MAX_ARMY_SIZE];
        uint tank1_pixss[MAX_ARMY_SIZE * (TANK_SPRITE_FRAME_SIZE - 1) * (TANK_SPRITE_FRAME_SIZE - 1)];
        int tank1_frames[MAX_ARMY_SIZE];
        uint* tank1_backups[MAX_ARMY_SIZE];
        uint* tank1_last_targets[MAX_ARMY_SIZE];
        int2 tank1_last_poss[MAX_ARMY_SIZE];

        uint next_tank2;
        // Updated by Tank::Tick()
        float2 tank2_poss[MAX_ARMY_SIZE];
        int tank2_frames[MAX_ARMY_SIZE];
        // Not updated by Tank::Tick()
        int2 tank2_int_poss[MAX_ARMY_SIZE];
        int tank2_x1s[MAX_ARMY_SIZE], tank2_x2s[MAX_ARMY_SIZE], tank2_y1s[MAX_ARMY_SIZE], tank2_y2s[MAX_ARMY_SIZE];
        uint tank2_frac_xs[MAX_ARMY_SIZE], tank2_frac_ys[MAX_ARMY_SIZE];
        uint tank2_interpol_weight_0s[MAX_ARMY_SIZE], tank2_interpol_weight_1s[MAX_ARMY_SIZE], tank2_interpol_weight_2s[
                 MAX_ARMY_SIZE], tank2_interpol_weight_3s[MAX_ARMY_SIZE];
        uint tank2_pixss[MAX_ARMY_SIZE * (TANK_SPRITE_FRAME_SIZE - 1) * (TANK_SPRITE_FRAME_SIZE - 1)];
        uint* tank2_backups[MAX_ARMY_SIZE];
        uint* tank2_last_targets[MAX_ARMY_SIZE];
        int2 tank2_last_poss[MAX_ARMY_SIZE];

        uint next_sand0;
        // For TickSand
        float2 sand0_poss[THIRD_MAX_SAND];
        float2 sand0_dirs[THIRD_MAX_SAND];
        uint sand0_colors[THIRD_MAX_SAND];
        uint sand0_frame_changes[THIRD_MAX_SAND];
        int sand0_frames[THIRD_MAX_SAND];
        // For DrawSprite
        int2 sand0_int_poss[THIRD_MAX_SAND];
        int sand0_x1s[THIRD_MAX_SAND], sand0_x2s[THIRD_MAX_SAND], sand0_y1s[THIRD_MAX_SAND], sand0_y2s[THIRD_MAX_SAND];
        uint sand0_frac_xs[THIRD_MAX_SAND], sand0_frac_ys[THIRD_MAX_SAND];
        uint sand0_interpol_weight_0s[THIRD_MAX_SAND], sand0_interpol_weight_1s[THIRD_MAX_SAND],
             sand0_interpol_weight_2s[THIRD_MAX_SAND], sand0_interpol_weight_3s[THIRD_MAX_SAND];
        uint sand0_pixss[THIRD_MAX_SAND * (BUSH_0_FRAME_SIZE - 1) * (BUSH_0_FRAME_SIZE - 1)];
        uint* sand0_backups[THIRD_MAX_SAND];
        uint* sand0_last_targets[THIRD_MAX_SAND];
        int2 sand0_last_poss[THIRD_MAX_SAND];

        uint next_sand1;
        // For TickSand
        float2 sand1_poss[THIRD_MAX_SAND];
        float2 sand1_dirs[THIRD_MAX_SAND];
        uint sand1_colors[THIRD_MAX_SAND];
        uint sand1_frame_changes[THIRD_MAX_SAND];
        int sand1_frames[THIRD_MAX_SAND];
        // For DrawSprite
        int2 sand1_int_poss[THIRD_MAX_SAND];
        int sand1_x1s[THIRD_MAX_SAND], sand1_x2s[THIRD_MAX_SAND], sand1_y1s[THIRD_MAX_SAND], sand1_y2s[THIRD_MAX_SAND];
        uint sand1_frac_xs[THIRD_MAX_SAND], sand1_frac_ys[THIRD_MAX_SAND];
        uint sand1_interpol_weight_0s[THIRD_MAX_SAND], sand1_interpol_weight_1s[THIRD_MAX_SAND],
             sand1_interpol_weight_2s[THIRD_MAX_SAND], sand1_interpol_weight_3s[THIRD_MAX_SAND];
        uint sand1_pixss[THIRD_MAX_SAND * (BUSH_1_FRAME_SIZE - 1) * (BUSH_1_FRAME_SIZE - 1)];
        uint* sand1_backups[THIRD_MAX_SAND];
        uint* sand1_last_targets[THIRD_MAX_SAND];
        int2 sand1_last_poss[THIRD_MAX_SAND];
        
        uint next_sand2;
        // For TickSand
        float2 sand2_poss[THIRD_MAX_SAND];
        float2 sand2_dirs[THIRD_MAX_SAND];
        uint sand2_colors[THIRD_MAX_SAND];
        uint sand2_frame_changes[THIRD_MAX_SAND];
        int sand2_frames[THIRD_MAX_SAND];
        // For DrawSprite
        int2 sand2_int_poss[THIRD_MAX_SAND];
        int sand2_x1s[THIRD_MAX_SAND], sand2_x2s[THIRD_MAX_SAND], sand2_y1s[THIRD_MAX_SAND], sand2_y2s[THIRD_MAX_SAND];
        uint sand2_frac_xs[THIRD_MAX_SAND], sand2_frac_ys[THIRD_MAX_SAND];
        uint sand2_interpol_weight_0s[THIRD_MAX_SAND], sand2_interpol_weight_1s[THIRD_MAX_SAND],
             sand2_interpol_weight_2s[THIRD_MAX_SAND], sand2_interpol_weight_3s[THIRD_MAX_SAND];
        uint sand2_pixss[THIRD_MAX_SAND * (BUSH_2_FRAME_SIZE - 1) * (BUSH_2_FRAME_SIZE - 1)];
        uint* sand2_backups[THIRD_MAX_SAND];
        uint* sand2_last_targets[THIRD_MAX_SAND];
        int2 sand2_last_poss[THIRD_MAX_SAND];

        /*
         * TickSand expects that each pointer argument is an array of shape [MAX_THIRD_SAND], and that total < THIRD_MAX_SAND. 
         */
        void TickSand(float2* poss, float2* dirs, int* frames, uint* frame_changes, uint total)
        {
            for (uint i = 0; i < total; i++)
            {
                poss[i] += dirs[i];
                dirs[i].y *= 0.95f;
                if (poss[i].x < 0)
                {
                    poss[i].x = (float)(Game::map.bitmap->width - 1);
                    poss[i].y = (float)(RandomUInt() % Game::map.bitmap->height);
                    dirs[i] = make_float2(-1 - RandomFloat() * 2, 0);
                }
                for (int s = (int)Game::peaks.size(), i = 0; i < s; i++)
                {
                    float2 toPeak = make_float2(Game::peaks[i].x, Game::peaks[i].y) - poss[i];
                    float g = Game::peaks[i].z * 0.02f / sqrtf(dot(toPeak, toPeak));
                    toPeak = normalize(toPeak);
                    dirs[i].y -= toPeak.y * g;
                }
                dirs[i].y += RandomFloat() * 0.05f - 0.025f;
                frames[i] = (frames[i] + frame_changes[i] + 256) & 255;
            }
        }

        /*
         * DrawSprite expects that each pointer argument is an  array of size 'total'. Except for:
         * - target is a pointer to the surface to draw to
         * - p0ss, p1ss, p2ss, p3ss, and pixss are multidimensional arrays of shape [total][s.frameSize - 1][s.frameSize - 1]
         *   projected to the shape [total * (s.frameSize - 1) * (s.frameSize - 1)]
         */
        void DrawSprite(
            Sprite s,
            float2* poss,
            int* frames,
            int2* int_poss,
            int* x1s, int* x2s, int* y1s, int* y2s,
            uint* frac_xs, uint* frac_ys,
            uint* interpol_weight_0s, uint* interpol_weight_1s, uint* interpol_weight_2s, uint* interpol_weight_3s,
            uint* pixss,
            uint** last_targets,
            int2* last_poss,
            uint** backups,
            Surface* target,
            uint total);
        /*
         * Each pointer argument should be an array of size 'total'.
         */
        void RemoveSprite(Sprite s, uint** backups, uint** last_targets, int2* last_poss, uint total);

        // static data, for global access
        static inline bool debug_print_frame_time;
        static inline bool is_tick_paused;
        static inline Map map; // the map
        static inline vector<Actor*> actorPool; // actor pool
        static inline vector<float3> peaks; // mountain peaks to evade
        static inline Grid grid; // actor grid for faster range queries
        static inline int coolDown = 0; // used to prevent simultaneous firing
    };
} // namespace Tmpl8
