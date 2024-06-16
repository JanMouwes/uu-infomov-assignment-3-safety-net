# Performance Progress

## Initial performance

Frame counts are stable after 15 seconds.

### Debug mode

Each frame takes 115-116 milliseconds.
FRAPS reports about 8-9 FPS.

### Release mode

Each frame takes 30-31 milliseconds.
FRAPS reports 30-31 FPS.

## Log

### 2024/06/06

uProf reported that in `SpriteInstance::Draw`, the line below was taking up a lot of
time. We figured out that this line was used for scaled sprites only. The use of bilinear
interpolation does not seem to have any significant visual impact on the game.
Hence, we disabled it through a pragma. This resulted in 26-27 ms per frame, which 
FRAPS reports as 34-35 FPS.

```cpp
dst[u] = ScaleColor(pix_colour_argb, alpha) + ScaleColor(dst[u], 255 - alpha);
```

### 2024/06/15

#### SpriteInstance::Draw

AMD uProf reports two major hotspots:
```
Line,Source,CPU_TIME(s)
219,"const uint p0 = ScaleColor(src[0], interpol_weight_0);",0.67%
225,"if (alpha) *dst = ScaleColor(pix, alpha) + ScaleColor(*dst, 255 - alpha);",1.08%
```
Reported average frame time after is approx 31ms.

AMD uProf also reports a lot of cache misses:
```
Line,Source,IBS_DC_MISS_LAT
238,"p3s[u] = ScaleColor(src[stride + 1], interpol_weight_3);",10033
```

Changing
```cpp
src = sprite->pixels + frame * sprite->frameSize + v * stride;
for (int u = 0; u < sprite->frameSize - 1; u++, src++)
{
    p2s[u] = ScaleColor(src[stride], interpol_weight_2);
    p3s[u] = ScaleColor(src[stride + 1], interpol_weight_3);
    pixs[u] = p0s[u] + p1s[u] + p2s[u] + p3s[u];
}
```
to
```cpp
src = sprite->pixels + frame * sprite->frameSize + v * stride;
for (int u = 0; u < sprite->frameSize - 1; u++, src++)
{
    p2s[u] = ScaleColor(src[stride], interpol_weight_2);
}

src = sprite->pixels + frame * sprite->frameSize + v * stride;
for (int u = 0; u < sprite->frameSize - 1; u++, src++)
{
    p3s[u] = ScaleColor(src[stride + 1], interpol_weight_3);
}

for (int u = 0; u < sprite->frameSize - 1; u++)
{
    pixs[u] = p0s[u] + p1s[u] + p2s[u] + p3s[u];
}
```
Improved cache-misses on that that operation, AMD uProf reports no `IBS_DC_MISS_LAT`
```
Line,Source,IBS_LOAD_STORE,IBS_LOAD,IBS_DC_MISS_LAT,IBS_NB_CACHE_MODIFIED,IBS_NB_LOCAL_CACHE_MODIFIED,IBS_NB_REMOTE_CACHE_MODIFIED,IBS_STORE,IBS_STORE_DC_MISS,IBS_NB_LOCAL_DRAM,IBS_NB_REMOTE_DRAM,IBS_NB_LOCAL_CACHE_OWNED,IBS_NB_REMOTE_CACHE_OWNED,IBS_NB_LOCAL_CACHE_MISS,IBS_LOAD_DC_L2_HIT
243,"p3s[u] = ScaleColor(src[stride + 1], interpol_weight_3);",2213,1142,,,,,1071,33,,,,,,1142
```
`IBS_DC_MISS_LAT` is the "Accumulated load latencies for the loads to cache lines" [AMD uProf User Guide](https://www.amd.com/content/dam/amd/en/documents/developer/uprof-v4.0-gaGA-user-guide.pdf).

Created a complete SoA style drawing flow for tank1 and tank2 sprites.

### 06/06/16

#### Map::Draw

uProf now reports that the majority of time spent in TankGame is Map::Draw:


```
Line,Source,CPU_TIME(s)
72,const uint p2 = mapLine[mapPos + 1]; // mem,0.53%
76,const uint p4 = mapLine[mapPos + width + 1]; // mem,0.53%
77,combined += p4; //int,0.47%
86,"*dst = ScaleColor( p1, w1 ) + ScaleColor( p2, w2 ) + ScaleColor( p3, w3 ) + ScaleColor( p4, w4 ); //",0.41%
```

Average frame time when the right before the tanks start shooting is 35ms.
FRAPS shows 27 FPS.

Removing:
```cpp
#pragma omp parallel for schedule(static)
```
Increases performance at that point to 29ms and 33 FPS.

#### Back to SpriteInstance::Draw

Now the majority of time is spent in `SpriteInstance::Draw` again (25%!).
Specifically:
```cpp
Line,Source,CPU_TIME(s)
225,"p0s[u] = ScaleColor(src[0], interpol_weight_0);",4.00%
231,"p1s[u] = ScaleColor(src[1], interpol_weight_1);",3.72%
237,"p2s[u] = ScaleColor(src[stride], interpol_weight_2);",3.40%
243,"p3s[u] = ScaleColor(src[stride + 1], interpol_weight_3);",3.28%
254,"if (alpha) *dst = ScaleColor(pixs[u], alpha) + ScaleColor(*dst, 255 - alpha);",3.34%
```

Particles (the sand/tumbleweed), bullets, explosions, and flashes all use SpriteInstance::Draw.
Since the profiling is done _before_ the tanks start shooting, the time is spent on drawing the sand.

Moving to a SoA style layout for sand's tick does not gain much in performance.
However it does mean that `57%` of CPU Time is spent in `Game::DrawSprite`.

#### Back to `Game::DrawSprite`

Specifically:
```
Line,Source,CPU_TIME(s)
437,"void Game::RemoveSprite(Sprite s, uint** backups, Surface** last_targets, int2* last_poss, uint total)",12.21%
413,"p2ss[i *  (s.frameSize - 1) + u] = ScaleColor(src[stride], interpol_weight_2s[i]);",8.69%
407,"p1ss[i *  (s.frameSize - 1) + u] = ScaleColor(src[1], interpol_weight_1s[i]);",7.41%
```

It is hard to split up the loop that loops over all of these statements, because it lacks the dimension that `v` introduces.
So, all p0ss, p1ss, p2ss, and pixss have an added dimensionality of v.

## Final performance