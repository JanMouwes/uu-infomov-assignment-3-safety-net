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
        float2 tank1_poss[SPRITE_SOA_SIZE];
        int tank1_frames[SPRITE_SOA_SIZE];
        uint* tank1_backups[SPRITE_SOA_SIZE];
        Surface* tank1_last_targets[SPRITE_SOA_SIZE];
        int2 tank1_last_poss[SPRITE_SOA_SIZE];

        uint next_tank2;
        float2 tank2_poss[SPRITE_SOA_SIZE];
        int tank2_frames[SPRITE_SOA_SIZE];
        uint* tank2_backups[SPRITE_SOA_SIZE];
        Surface* tank2_last_targets[SPRITE_SOA_SIZE];
        int2 tank2_last_poss[SPRITE_SOA_SIZE];
        void DrawSprite(Sprite s, float2 poss[SPRITE_SOA_SIZE], int frames[SPRITE_SOA_SIZE], Surface* target,
                        uint* backups[SPRITE_SOA_SIZE], Surface* last_targets[SPRITE_SOA_SIZE],
                        int2 last_poss[SPRITE_SOA_SIZE], uint total);
        void RemoveSprite(Sprite s, uint* backups[SPRITE_SOA_SIZE], Surface* last_targets[SPRITE_SOA_SIZE],
                          int2 last_poss[SPRITE_SOA_SIZE], uint total);

        // static data, for global access
        static inline bool debug_print_frame_time;
        static inline bool is_tick_paused;
        static inline Map map; // the map
        static inline vector<Actor*> actorPool; // actor pool
        static inline vector<float3> peaks; // mountain peaks to evade
        static inline vector<Particle*> sand; // sand particles
        static inline Grid grid; // actor grid for faster range queries
        static inline int coolDown = 0; // used to prevent simultaneous firing

        void Draw8(SpriteInstance s[8], Surface* target, float2 pos8[8], int frame8[8])
        {
            uint pixs[8][1024];

            for (int i = 0; i < 8; i++)
            {
                Sprite* sprite = s[i].sprite;
                uint* backup = s[i].backup;
                float2 pos = pos8[i];
                int frame = frame8[i];
                // Will RemoveSprite see changes to these?
                Surface* lastTarget = s[i].lastTarget;
                int2 lastPos = s[i].lastPos;

                int frameSize = sprite->frameSize;

                // save the area of target that we are about to overwrite
                if (!backup) backup = new uint[sqr(sprite->frameSize + 1)];
                const int2 intPos = make_int2(pos);
                int x1 = intPos.x - sprite->frameSize / 2, x2 = x1 + sprite->frameSize;
                int y1 = intPos.y - sprite->frameSize / 2, y2 = y1 + sprite->frameSize;

                if (x1 < 0 || y1 < 0 || x2 >= target->width || y2 >= target->height)
                {
                    // out of range; skip
                    lastTarget = 0;
                    return;
                }
                for (int v = 0; v < sprite->frameSize; v++)
                    memcpy(backup + v * sprite->frameSize,
                           target->pixels + x1 + (y1 + v) * target->width,
                           sprite->frameSize * 4);
                lastPos = make_int2(x1, y1);
                lastTarget = target;
                // calculate bilinear weights - these are constant in this case.
                const uint frac_x = (int)(255.0f * (pos.x - floorf(pos.x)));
                const uint frac_y = (int)(255.0f * (pos.y - floorf(pos.y)));
                const uint interpol_weight_0 = (frac_x * frac_y) >> 8;
                const uint interpol_weight_1 = ((255 - frac_x) * frac_y) >> 8;
                const uint interpol_weight_2 = (frac_x * (255 - frac_y)) >> 8;
                const uint interpol_weight_3 = ((255 - frac_x) * (255 - frac_y)) >> 8;
                // draw the sprite frame
                const uint stride = sprite->frameCount * sprite->frameSize;

                // top-to-bottom
                for (int v = 0; v < sprite->frameSize - 1; v++)
                {
                    uint* dst = target->pixels + x1 + (y1 + v) * target->width;
                    {
                        // fallback
                        uint* src = sprite->pixels + frame * sprite->frameSize + v * stride;
                        for (int u = 0; u < frameSize - 1; u++, src++)
                        {
                            pixs[i][u] = ScaleColor(src[0], interpol_weight_0);
                            pixs[i][u] += ScaleColor(src[1], interpol_weight_1);
                        }

                        src = sprite->pixels + frame * frameSize + v * stride;
                        for (int u = 0; u < sprite->frameSize - 1; u++, src++)
                        {
                            pixs[i][u] += ScaleColor(src[stride], interpol_weight_2);
                            pixs[i][u] += ScaleColor(src[stride + 1], interpol_weight_3);
                        }

                        for (int u = 0; u < frameSize - 1; u++, dst++)
                        {
                            uint alpha = pixs[i][u] >> 24;
                            if (alpha) *dst = ScaleColor(pixs[i][u], alpha) + ScaleColor(*dst, 255 - alpha);
                        }
                    }
                }
            }
        }
    };
} // namespace Tmpl8
