#include "precomp.h"
#include "game.h"

#define TANK1_FRAME_SIZE 36
#define TANK1_FRAME 1

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
    // load tank sprites
    tank1 = new Sprite("assets/tanks.png", make_int2(128, 100), make_int2(310, 360), 36, 256);
    tank2 = new Sprite("assets/tanks.png", make_int2(327, 99), make_int2(515, 349), 36, 256);
    // load bush sprite for dust streams
    bush[0] = new Sprite("assets/bush1.png", make_int2(2, 2), make_int2(31, 31), 10, 256);
    bush[1] = new Sprite("assets/bush2.png", make_int2(2, 2), make_int2(31, 31), 14, 256);
    bush[2] = new Sprite("assets/bush3.png", make_int2(2, 2), make_int2(31, 31), 20, 256);
    bush[0]->ScaleAlpha(96);
    bush[1]->ScaleAlpha(64);
    bush[2]->ScaleAlpha(128);
    // pointer
    pointer = new SpriteInstance(new Sprite("assets/pointer.png"));
    // create armies
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++) // main groups
        {
            Actor* army1Tank = new Tank(tank1, make_int2(520 + x * 32, 2420 - y * 32), make_int2(5000, -500), 0, 0);
            Actor* army2Tank = new Tank(tank2, make_int2(3300 - x * 32, y * 32 + 700), make_int2(-1000, 4000), 10, 1);
            actorPool.push_back(army1Tank);
            actorPool.push_back(army2Tank);
        }
    for (int y = 0; y < 12; y++)
        for (int x = 0; x < 12; x++) // backup
        {
            Actor* army1Tank = new Tank(tank1, make_int2(40 + x * 32, 2620 - y * 32), make_int2(5000, -500), 0, 0);
            Actor* army2Tank = new Tank(tank2, make_int2(3900 - x * 32, y * 32 + 300), make_int2(-1000, 4000), 10, 1);
            actorPool.push_back(army1Tank);
            actorPool.push_back(army2Tank);
        }
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) // small forward groups
        {
            Actor* army1Tank = new Tank(tank1, make_int2(1440 + x * 32, 2220 - y * 32), make_int2(3500, -500), 0, 0);
            Actor* army2Tank = new Tank(tank2, make_int2(2400 - x * 32, y * 32 + 900), make_int2(1300, 4000), 128, 1);
            actorPool.push_back(army1Tank);
            actorPool.push_back(army2Tank);
        }
    // load mountain peaks
    Surface mountains("assets/peaks.png");
    for (int y = 0; y < mountains.height; y++)
        for (int x = 0; x < mountains.width; x++)
        {
            uint p = mountains.pixels[x + y * mountains.width];
            if ((p & 0xffff) == 0) peaks.push_back(make_float3(make_int3(x * 8, y * 8, (p >> 16) & 255)));
        }
    // add sandstorm
    for (int i = 0; i < 7500; i++)
    {
        int x = RandomUInt() % map.bitmap->width;
        int y = RandomUInt() % map.bitmap->height;
        int d = (RandomUInt() & 15) - 8;
        sand.push_back(new Particle(bush[i % 3], make_int2(x, y), map.bitmap->pixels[x + y * map.bitmap->width], d));
    }
    // place flags
    Surface* flagPattern = new Surface("assets/flag.png");
    VerletFlag* flag1 = new VerletFlag(make_int2(3000, 848), flagPattern);
    actorPool.push_back(flag1);
    VerletFlag* flag2 = new VerletFlag(make_int2(1076, 1870), flagPattern);
    actorPool.push_back(flag2);
    // initialize map view
    map.UpdateView(screen, zoom);
}

// -----------------------------------------------------------
// Advanced zooming
// -----------------------------------------------------------
void Game::MouseWheel(float y)
{
    // fetch current pointer location
    int2 pointerPos = map.ScreenToMap(mousePos);
    // adjust zoom
    zoom -= 10 * y;
    if (zoom < 20) zoom = 20;
    if (zoom > 100) zoom = 100;
    // adjust focus so that pointer remains stationary, if possible
    map.UpdateView(screen, zoom);
    int2 newPointerPos = map.ScreenToMap(mousePos);
    map.SetFocus(map.GetFocus() + (pointerPos - newPointerPos));
    map.UpdateView(screen, zoom);
}

// -----------------------------------------------------------
// Process mouse input
// -----------------------------------------------------------
void Game::HandleInput()
{
    // anything that happens only once at application start goes here
    static bool wasDown = false, dragging = false;
    if (mouseDown && !wasDown) dragging = true, dragStart = mousePos, focusStart = map.GetFocus();
    if (!mouseDown) dragging = false;
    wasDown = mouseDown;
    if (dragging)
    {
        int2 delta = dragStart - mousePos;
        delta.x = (int)((delta.x * zoom) / 32);
        delta.y = (int)((delta.y * zoom) / 32);
        map.SetFocus(focusStart + delta);
        map.UpdateView(screen, zoom);
    }
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
    Timer t;
    // draw the map
    map.Draw(screen);
    // rebuild actor grid
    grid.Clear();
    grid.Populate(actorPool);
    // update and render actors
    pointer->Remove();
    for (int s = (int)sand.size(), i = s - 1; i >= 0; i--) sand[i]->Remove();
    RemoveSprite(*tank2, tank2_backups, tank2_last_targets, tank2_last_poss, next_tank2);
    RemoveSprite(*tank1, tank1_backups, tank1_last_targets, tank1_last_poss, next_tank1);
    for (int s = (int)actorPool.size(), i = s - 1; i >= 0; i--)
    {
        if (actorPool[i]->GetType() == Actor::TANK) continue;
        actorPool[i]->Remove();
    }
    if (!is_tick_paused)
    {
        for (int s = (int)sand.size(), i = 0; i < s; i++) sand[i]->Tick();
        for (int i = 0; i < (int)actorPool.size(); i++)
            if (!actorPool[i]->Tick())
            {
                // actor got deleted, replace by last in list
                Actor* lastActor = actorPool.back();
                Actor* toDelete = actorPool[i];
                actorPool.pop_back();
                if (lastActor != toDelete) actorPool[i] = lastActor;
                delete toDelete;
                i--;
            }
        coolDown++;
    }
    next_tank1 = 0;
    next_tank2 = 0;
    for (int s = (int)actorPool.size(), i = 0; i < s; i++)
    {
        if (actorPool[i]->GetType() == Actor::TANK && ((Tank*)actorPool[i])->army == 0)
        {
            assert(next_tank1 < SPRITE_SOA_SIZE);
            tank1_poss[next_tank1] = actorPool[i]->pos;
            tank1_frames[next_tank1] = actorPool[i]->frame;
            next_tank1++;
            continue;
        }
        if (actorPool[i]->GetType() == Actor::TANK && ((Tank*)actorPool[i])->army == 1)
        {
            assert(next_tank2 < SPRITE_SOA_SIZE);
            tank2_poss[next_tank2] = actorPool[i]->pos;
            tank2_frames[next_tank2] = actorPool[i]->frame;
            next_tank2++;
            continue;
        }
        actorPool[i]->Draw();
    }

    DrawSprite(*tank1, tank1_poss, tank1_frames, map.bitmap, tank1_backups, tank1_last_targets, tank1_last_poss, next_tank1);
    DrawSprite(*tank2, tank2_poss, tank2_frames, map.bitmap, tank2_backups, tank2_last_targets, tank2_last_poss, next_tank2);

    for (int s = (int)sand.size(), i = 0; i < s; i++) sand[i]->Draw();
    int2 cursorPos = map.ScreenToMap(mousePos);
    pointer->Draw(map.bitmap, make_float2(cursorPos), 0);
    // handle mouse
    HandleInput();
    // report frame time
    static float frameTimeAvg = 10.0f; // estimate
    frameTimeAvg = 0.95f * frameTimeAvg + 0.05f * t.elapsed() * 1000;
    if (debug_print_frame_time)
    {
        printf("frame time: %5.2fms\n", frameTimeAvg);
    }
}

void Game::DrawSprite(Sprite s, float2 poss[SPRITE_SOA_SIZE], int frames[SPRITE_SOA_SIZE], Surface* target, uint* backups[SPRITE_SOA_SIZE], Surface* last_targets[SPRITE_SOA_SIZE], int2 last_poss[SPRITE_SOA_SIZE], uint total)
{
    assert(total < SPRITE_SOA_SIZE);
    for (uint i = 0; i < total; i++)
    {
        if (!backups[i]) backups[i] = new uint[(s.frameSize + 1) * (s.frameSize + 1)];
    }

    int2 intPoss[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        intPoss[i] = make_int2(poss[i]);
    }

    int x1s[SPRITE_SOA_SIZE], x2s[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        x1s[i] = intPoss[i].x - s.frameSize / 2;
        x2s[i] = x1s[i] + s.frameSize;
    }
    int y1s[SPRITE_SOA_SIZE], y2s[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        y1s[i] = intPoss[i].y - s.frameSize / 2;
        y2s[i] = y1s[i] + s.frameSize;
    }

    for (uint i = 0; i < total; i++)
    {
        if (x1s[i] < 0 || y1s[i] < 0 || x2s[i] >= target->width || y2s[i] >= target->height)
        {
            last_targets[i] = 0;
        }
        else
        {
            last_targets[i] = target;
        }
    }

    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        for (int v = 0; v < s.frameSize; v++)
            memcpy(backups[i] + v * s.frameSize,
                   target->pixels + x1s[i] + (y1s[i] + v) * target->width,
                   s.frameSize * 4);
    }

    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        last_poss[i] = make_int2(x1s[i], y1s[i]);
    }
    
    uint frac_xs[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        frac_xs[i] = (int)(255.0f * (poss[i].x - floorf(poss[i].x)));
    }

    uint frac_ys[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        frac_ys[i] = (int)(255.0f * (poss[i].y - floorf(poss[i].y)));
    }

    uint interpol_weight_0s[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        interpol_weight_0s[i] = (frac_xs[i] * frac_ys[i]) >> 8;
    }

    uint interpol_weight_1s[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        interpol_weight_1s[i] = ((255 - frac_xs[i]) * frac_ys[i]) >> 8;
    }

    uint interpol_weight_2s[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        interpol_weight_2s[i] = (frac_xs[i] * (255 - frac_ys[i])) >> 8;
    }

    uint interpol_weight_3s[SPRITE_SOA_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        interpol_weight_3s[i] = ((255 - frac_xs[i]) * (255 - frac_ys[i])) >> 8;
    }

    const uint stride = s.frameCount * s.frameSize;

    // TODO: frameSize is hardcoded for the tank sprites.
    uint p0ss[SPRITE_SOA_SIZE][TANK1_FRAME_SIZE];
    uint p1ss[SPRITE_SOA_SIZE][TANK1_FRAME_SIZE];
    uint p2ss[SPRITE_SOA_SIZE][TANK1_FRAME_SIZE];
    uint p3ss[SPRITE_SOA_SIZE][TANK1_FRAME_SIZE];
    uint pixss[SPRITE_SOA_SIZE][TANK1_FRAME_SIZE];
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        for (int v = 0; v < s.frameSize - 1; v++)
        {
            uint* src = s.pixels + frames[i] * s.frameSize + v * stride;
            for (int u = 0; u < s.frameSize - 1; u++, src++)
            {
                p0ss[i][u] = ScaleColor(src[0], interpol_weight_0s[i]);
            }

            src = s.pixels + frames[i] * s.frameSize + v * stride;
            for (int u = 0; u < s.frameSize - 1; u++, src++)
            {
                p1ss[i][u] = ScaleColor(src[1], interpol_weight_1s[i]);
            }

            src = s.pixels + frames[i] * s.frameSize + v * stride;
            for (int u = 0; u < s.frameSize - 1; u++, src++)
            {
                p2ss[i][u] = ScaleColor(src[stride], interpol_weight_2s[i]);
            }

            src = s.pixels + frames[i] * s.frameSize + v * stride;
            for (int u = 0; u < s.frameSize - 1; u++, src++)
            {
                p3ss[i][u] = ScaleColor(src[stride + 1], interpol_weight_3s[i]);
            }

            for (int u = 0; u < s.frameSize - 1; u++)
            {
                pixss[i][u] = p0ss[i][u] + p1ss[i][u] + p2ss[i][u] + p3ss[i][u];
            }

            uint* dst = target->pixels + x1s[i] + (y1s[i] + v) * target->width;
            for (int u = 0; u < s.frameSize - 1; u++, dst++)
            {
                uint alpha = pixss[i][u] >> 24;
                if (alpha) *dst = ScaleColor(pixss[i][u], alpha) + ScaleColor(*dst, 255 - alpha);
            }
        }
    }
}

void Game::RemoveSprite(Sprite s, uint* backups[SPRITE_SOA_SIZE], Surface* last_targets[SPRITE_SOA_SIZE], int2 last_poss[SPRITE_SOA_SIZE], uint total)
{
    assert(total < SPRITE_SOA_SIZE);
    for (int i = total - 1; i >= 0; i--)
    {
        // use the stored pixels to restore the rectangle affected by the sprite.
        // note: sprites must be removed in reverse order to guarantee correct removal.
        if (last_targets[i])
            for (int v = 0; v < s.frameSize; v++)
            {
                memcpy(last_targets[i]->pixels + last_poss[i].x + (last_poss[i].y + v) * last_targets[i]->width,
                       backups[i] + v * s.frameSize, s.frameSize * 4);
            }
    }
}
