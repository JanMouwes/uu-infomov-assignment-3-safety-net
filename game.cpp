#include "precomp.h"
#include "game.h"

static Kernel *computeBoundingBoxes, *computeInterpolWeights;
static Buffer *poss_buffer, *bounding_box_buffer, *last_poss_buffer, interpol_weights_buffer;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
    Kernel::InitCL();
    computeBoundingBoxes = new Kernel("cl/program.cl", "computeBoundingBoxes");
    computeInterpolWeights = new Kernel(computeBoundingBoxes->GetProgram(), "computeInterpolWeights");

    poss_buffer = new Buffer(THIRD_MAX_SAND * 2 * sizeof(float));
    poss_buffer->CopyFromDevice();
    
    bounding_box_buffer = new Buffer(THIRD_MAX_SAND * 4 * sizeof(int));
    bounding_box_buffer->CopyFromDevice();
    
    last_poss_buffer = new Buffer(THIRD_MAX_SAND * 2 * sizeof (float));
    last_poss_buffer->CopyFromDevice();
    
    // load tank sprites
    tank1 = new Sprite("assets/tanks.png", make_int2(128, 100), make_int2(310, 360), TANK_SPRITE_FRAME_SIZE, TANK_SPRITE_FRAMES);
    tank2 = new Sprite("assets/tanks.png", make_int2(327, 99), make_int2(515, 349), TANK_SPRITE_FRAME_SIZE, TANK_SPRITE_FRAMES);
    // load bush sprite for dust streams
    bush[0] = new Sprite("assets/bush1.png", make_int2(2, 2), make_int2(31, 31), BUSH_0_FRAME_SIZE, BUSH_0_FRAMES);
    bush[1] = new Sprite("assets/bush2.png", make_int2(2, 2), make_int2(31, 31), BUSH_1_FRAME_SIZE, BUSH_1_FRAMES);
    bush[2] = new Sprite("assets/bush3.png", make_int2(2, 2), make_int2(31, 31), BUSH_2_FRAME_SIZE, BUSH_2_FRAMES);
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

    for (int i = 0; i < MAX_SAND; i++)
    {
        int x = RandomUInt() % map.bitmap->width;
        int y = RandomUInt() % map.bitmap->height;
        int d = (RandomUInt() & 15) - 8;

        if (i % 3 == 0)
        {
            assert(next_sand0 < THIRD_MAX_SAND);
            sand0_poss[next_sand0] = make_float2(x, y);
            sand0_dirs[next_sand0] = make_float2(-1 - RandomFloat() * 4, 0);
            sand0_colors[next_sand0] = map.bitmap->pixels[x + y * map.bitmap->width];
            sand0_frame_changes[next_sand0] = d;
            next_sand0++;
        }
        else if (i % 3 == 1)
        {
            assert(next_sand1 < THIRD_MAX_SAND);
            sand1_poss[next_sand1] = make_float2(x, y);
            sand1_dirs[next_sand1] = make_float2(-1 - RandomFloat() * 4, 0);
            sand1_colors[next_sand1] = map.bitmap->pixels[x + y * map.bitmap->width];
            sand1_frame_changes[next_sand1] = d;
            next_sand1++;
        }
        else
        {
            assert(next_sand2 < THIRD_MAX_SAND); sand2_poss[next_sand2] = make_float2(x, y);
            sand2_dirs[next_sand2] = make_float2(-1 - RandomFloat() * 4, 0);
            sand2_colors[next_sand2] = map.bitmap->pixels[x + y * map.bitmap->width];
            sand2_frame_changes[next_sand2] = d;
            next_sand2++;
        }
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
    RemoveSprite(*bush[2], sand2_backups, sand2_last_targets, sand2_last_poss, next_sand2);
    RemoveSprite(*bush[1], sand1_backups, sand1_last_targets, sand1_last_poss, next_sand1);
    RemoveSprite(*bush[0], sand0_backups, sand0_last_targets, sand0_last_poss, next_sand0);
    RemoveSprite(*tank2, tank2_backups, tank2_last_targets, tank2_last_poss, next_tank2);
    RemoveSprite(*tank1, tank1_backups, tank1_last_targets, tank1_last_poss, next_tank1);
    for (int s = (int)actorPool.size(), i = s - 1; i >= 0; i--)
    {
        if (actorPool[i]->GetType() == Actor::TANK) continue;
        actorPool[i]->Remove();
    }
    if (!is_tick_paused)
    {
        TickSand(sand0_poss, sand0_dirs, sand0_frames, sand0_frame_changes, next_sand0);
        TickSand(sand1_poss, sand1_dirs, sand1_frames, sand1_frame_changes, next_sand1);
        TickSand(sand2_poss, sand2_dirs, sand2_frames, sand2_frame_changes, next_sand2);
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

    // Construct SoA for drawing after physics has done its thing
    next_tank1 = 0;
    next_tank2 = 0;
    for (int s = (int)actorPool.size(), i = 0; i < s; i++)
    {
        if (actorPool[i]->GetType() == Actor::TANK && ((Tank*)actorPool[i])->army == 0)
        {
            assert(next_tank1 < MAX_ARMY_SIZE);
            tank1_poss[next_tank1] = actorPool[i]->pos;
            tank1_frames[next_tank1] = actorPool[i]->frame;
            next_tank1++;
            continue;
        }
        if (actorPool[i]->GetType() == Actor::TANK && ((Tank*)actorPool[i])->army == 1)
        {
            assert(next_tank2 < MAX_ARMY_SIZE);
            tank2_poss[next_tank2] = actorPool[i]->pos;
            tank2_frames[next_tank2] = actorPool[i]->frame;
            next_tank2++;
            continue;
        }
        actorPool[i]->Draw();
    }

    DrawSprite(
        *tank1,
        tank1_poss,
        tank1_frames,
        tank1_int_poss,
        tank1_x1s, tank1_x2s, tank1_y1s, tank1_y2s,
        tank1_frac_xs, tank1_frac_ys,
        tank1_interpol_weight_0s, tank1_interpol_weight_1s, tank1_interpol_weight_2s, tank1_interpol_weight_3s,
        tank1_pixss,
        tank1_last_targets,
        tank1_last_poss,
        tank1_backups,
        map.bitmap,
        next_tank1
    );
    DrawSprite(
        *tank2,
        tank2_poss,
        tank2_frames,
        tank2_int_poss,
        tank2_x1s, tank2_x2s, tank2_y1s, tank2_y2s,
        tank2_frac_xs, tank2_frac_ys,
        tank2_interpol_weight_0s, tank2_interpol_weight_1s, tank2_interpol_weight_2s, tank2_interpol_weight_3s,
        tank2_pixss,
        tank2_last_targets,
        tank2_last_poss,
        tank2_backups,
        map.bitmap,
        next_tank2
    );

    DrawSprite(
        *bush[0],
        sand0_poss,
        sand0_frames,
        sand0_int_poss,
        sand0_x1s, sand0_x2s, sand0_y1s, sand0_y2s,
        sand0_frac_xs, sand0_frac_ys,
        sand0_interpol_weight_0s, sand0_interpol_weight_1s, sand0_interpol_weight_2s, sand0_interpol_weight_3s,
        sand0_pixss,
        sand0_last_targets,
        sand0_last_poss,
        sand0_backups,
        map.bitmap,
        next_sand0
    );
    DrawSprite(
        *bush[1],
        sand1_poss,
        sand1_frames,
        sand1_int_poss,
        sand1_x1s, sand1_x2s, sand1_y1s, sand1_y2s,
        sand1_frac_xs, sand1_frac_ys,
        sand1_interpol_weight_0s, sand1_interpol_weight_1s, sand1_interpol_weight_2s, sand1_interpol_weight_3s,
        sand1_pixss,
        sand1_last_targets,
        sand1_last_poss,
        sand1_backups,
        map.bitmap,
        next_sand1
    );
    DrawSprite(
        *bush[2],
        sand2_poss,
        sand2_frames,
        sand2_int_poss,
        sand2_x1s, sand2_x2s, sand2_y1s, sand2_y2s,
        sand2_frac_xs, sand2_frac_ys,
        sand2_interpol_weight_0s, sand2_interpol_weight_1s, sand2_interpol_weight_2s, sand2_interpol_weight_3s,
        sand2_pixss,
        sand2_last_targets,
        sand2_last_poss,
        sand2_backups,
        map.bitmap,
        next_sand2
    );
    
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

void Game::DrawSprite(
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
    uint total
)
{
    uint stride = s.frameCount * s.frameSize;

    // TODO: Move to initialization
    for (uint i = 0; i < total; i++)
    {
        if (!backups[i]) backups[i] = new uint[(s.frameSize + 1) * (s.frameSize + 1)];
    }
    // END TODO

    // Start Prepare data for the GPU
    // poss_buffer is a float2[THIRD_MAX_SAND], assumes that total < THIRD_MAX_SAND
    float *host_poss = (float*) poss_buffer->GetHostPtr();
    for (uint i = 0; i < total; i++)
    {
        host_poss[2 * i + 0] = poss[i].x;
        host_poss[2 * i + 1] = poss[i].y;
    }
    poss_buffer->CopyToDevice();
    // End prepare data for the GPU
    
    computeBoundingBoxes->SetArguments(
        poss_buffer,
        s.frameSize,
        bounding_box_buffer,
        last_poss_buffer);
    computeBoundingBoxes->Run(total);

    // Get the results from the GPU
    bounding_box_buffer->CopyFromDevice();
    int *bounding_boxes = (int*)bounding_box_buffer->GetHostPtr();

    for (uint i = 0; i < total; i++)
    {
        if (bounding_boxes[4 * i + 0] < 0 || bounding_boxes[4 * i + 2] < 0 || bounding_boxes[4 * i + 1] >= MAPWIDTH || bounding_boxes[4 * i + 3] >= MAPHEIGHT)
        {
            last_targets[i] = 0;
        }
        else
        {
            last_targets[i] = target->pixels;
            for (int v = 0; v < s.frameSize; v++)
            {
                uint* dst = backups[i] + v * s.frameSize;
                uint* src = target->pixels + bounding_boxes[4 * i + 0] + (bounding_boxes[4 * i + 2] + v) * MAPWIDTH;
                memcpy(dst, src, s.frameSize * 4);
            }
        }
    }

    // Get the results from the GPU
    // Convert them to something that the CPU can use because of RemoveSprite
    last_poss_buffer->CopyFromDevice();
    int* last_poss_result = (int*)last_poss_buffer->GetHostPtr();
    for (int i = 0; i < total; i++)
    {
        last_poss[i] = make_int2(last_poss_result[i * 2 + 0], last_poss_result[i * 2 + 1]);
    }
    
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;
        frac_xs[i] = (int)(255.0f * (poss[i].x - floorf(poss[i].x)));
        frac_ys[i] = (int)(255.0f * (poss[i].y - floorf(poss[i].y)));
        
        interpol_weight_0s[i] = (frac_xs[i] * frac_ys[i]) >> 8;
        interpol_weight_1s[i] = ((255 - frac_xs[i]) * frac_ys[i]) >> 8;
        interpol_weight_2s[i] = (frac_xs[i] * (255 - frac_ys[i])) >> 8;
        interpol_weight_3s[i] = ((255 - frac_xs[i]) * (255 - frac_ys[i])) >> 8;
    }

    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;

        for (int v = 0; v < s.frameSize - 1; v++)
        {
            const uint row_origin = frames[i] * s.frameSize + v * stride;
            const uint* src0 = s.scaledPixels[interpol_weight_0s[i]] + row_origin;
            const uint* src1 = s.scaledPixels[interpol_weight_1s[i]] + row_origin;

            for (int u = 0; u < s.frameSize - 1; u++)
            {
                pixss[To1D(u, v, i, s.frameSize - 1)] = src0[u];
                pixss[To1D(u, v, i, s.frameSize - 1)] += src1[u + 1];
            }
        }
    }

    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;

        for (int v = 0; v < s.frameSize - 1; v++)
        {
            const uint row_origin = frames[i] * s.frameSize + v * stride;
            const uint* src2 = s.scaledPixels[interpol_weight_2s[i]] + row_origin;
            const uint* src3 = s.scaledPixels[interpol_weight_3s[i]] + row_origin;

            for (int u = 0; u < s.frameSize - 1; u++)
            {
                pixss[To1D(u, v, i, s.frameSize - 1)] += src2[u + stride];
                pixss[To1D(u, v, i, s.frameSize - 1)] += src3[u + stride + 1];
            }
        }
    }

    // Get the results from the GPU
    for (uint i = 0; i < total; i++)
    {
        if (last_targets[i] == 0) continue;

        for (int v = 0; v < s.frameSize - 1; v++)
        {
            //                              x1s[i]            y1s[i]
            uint* dst = target->pixels + bounding_boxes[i * 4 + 0] + (bounding_boxes[i * 4 + 2] + v) * MAPWIDTH;
            for (int u = 0; u < s.frameSize - 1; u++)
            {
                const uint color = pixss[To1D(u, v, i, s.frameSize - 1)];
                const uint alpha = color >> 24;
                dst[u] = ScaleColor(color, alpha) + ScaleColor(dst[u], 255 - alpha);
            }
        }
    }
}

void Game::RemoveSprite(Sprite s, uint** backups, uint** last_targets, int2* last_poss, uint total)
{
    for (int i = total - 1; i >= 0; i--)
    {
        // use the stored pixels to restore the rectangle affected by the sprite.
        // note: sprites must be removed in reverse order to guarantee correct removal.
        if (last_targets[i])
            for (int v = 0; v < s.frameSize; v++)
            {
                memcpy(last_targets[i] + last_poss[i].x + (last_poss[i].y + v) * MAPWIDTH,
                       backups[i] + v * s.frameSize, s.frameSize * 4);
            }
    }
}
