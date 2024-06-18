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

### In the meantime

A bunch of trying to do full blown ECS....

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

Splitting each of the above statements into their own outer-loops means that FPS hovers around 22-24 FPS and ms stablises to 42ms.
AMD uProf shows the following hotspots:
```
Line,Source,CPU_TIME(s)
445,"if (alpha) *dst = ScaleColor(pixss[To1D(u, v, i, s.frameSize - 1)], alpha) + ScaleColor(*dst, 255 - alpha);",22.00%
401,"p0ss[To1D(u, v, i, s.frameSize - 1)] = ScaleColor(src[0], interpol_weight_0s[i]);",7.54%
420,"p2ss[To1D(u, v, i, s.frameSize - 1)] = ScaleColor(src[stride], interpol_weight_2s[i]);",7.34%
426,"p3ss[To1D(u, v, i, s.frameSize - 1)] = ScaleColor(src[stride + 1], interpol_weight_3s[i]);",6.18%
431,"pixss[To1D(u, v, i, s.frameSize - 1)] = p0ss[To1D(u, v, i, s.frameSize - 1)] + p1ss[To1D(u, v, i, s.frameSize - 1)] + p2ss[To1D(u, v, i, s.frameSize - 1)] + p3ss[To1D(u, v, i, s.frameSize - 1)];",4.42%
407,"p1ss[To1D(u, v, i, s.frameSize - 1)] = ScaleColor(src[1], interpol_weight_1s[i]);",6.28%
```
Where `To1D` is defined as
```cpp 
inline uint To1D(uint x, uint y, uint z, uint width, uint height)
{
    return z * width * height + y * width + x;
}

inline uint To1D(uint x, uint y, uint z, uint square)
{
    return To1D(x, y, z, square, square);
}
```

Investigating the cache performance in this area of the program shows that:

```
Line,Source,IBS_LOAD_STORE,IBS_LOAD,IBS_DC_MISS_LAT,IBS_NB_CACHE_MODIFIED,IBS_NB_LOCAL_CACHE_MODIFIED,IBS_NB_REMOTE_CACHE_MODIFIED,IBS_STORE,IBS_STORE_DC_MISS,IBS_NB_LOCAL_DRAM,IBS_NB_REMOTE_DRAM,IBS_NB_LOCAL_CACHE_OWNED,IBS_NB_REMOTE_CACHE_OWNED,IBS_NB_LOCAL_CACHE_MISS,IBS_LOAD_DC_L2_HIT
428,"pixss[To1D(u, v, i, s.frameSize - 1)] = p0ss[To1D(u, v, i, s.frameSize - 1)] + p1ss[To1D(u, v, i, s.frameSize - 1)] + p2ss[To1D(u, v, i, s.frameSize - 1)] + p3ss[To1D(u, v, i, s.frameSize - 1)];",22910,18347,52072,27,27,,4563,205,144,,,,144,18176
```

Moving the update of pixss to where the loops where p0ss and p1ss, and p2ss and p3ss are just read improves the cache performance:
```
Line,Source,IBS_LOAD_STORE,IBS_LOAD,IBS_DC_MISS_LAT,IBS_NB_CACHE_MODIFIED,IBS_NB_LOCAL_CACHE_MODIFIED,IBS_NB_REMOTE_CACHE_MODIFIED,IBS_STORE,IBS_STORE_DC_MISS,IBS_NB_LOCAL_DRAM,IBS_NB_REMOTE_DRAM,IBS_NB_LOCAL_CACHE_OWNED,IBS_NB_REMOTE_CACHE_OWNED,IBS_NB_LOCAL_CACHE_MISS,IBS_LOAD_DC_L2_HIT
418,"pixss[To1D(u, v, i, s.frameSize - 1)] += p2ss[To1D(u, v, i, s.frameSize - 1)] + p3ss[To1D(u, v, i, s.frameSize - 1)];",8467,8467,2911,2,2,,3709,5,10,,,,10,8455
403,"pixss[To1D(u, v, i, s.frameSize - 1)] = p0ss[To1D(u, v, i, s.frameSize - 1)] + p1ss[To1D(u, v, i, s.frameSize - 1)];",8314,4723,3599,,,,3591,31,10,,1,,10,4712
```

Total IBS_DC_MISS_LAT ~= 6.5k vs. ~52k

AMD uProf now shows cache contention hotspot:
```
Line,Source,IBS_LOAD_STORE,IBS_LOAD,IBS_DC_MISS_LAT,IBS_NB_CACHE_MODIFIED,IBS_NB_LOCAL_CACHE_MODIFIED,IBS_NB_REMOTE_CACHE_MODIFIED,IBS_STORE,IBS_STORE_DC_MISS,IBS_NB_LOCAL_DRAM,IBS_NB_REMOTE_DRAM,IBS_NB_LOCAL_CACHE_OWNED,IBS_NB_REMOTE_CACHE_OWNED,IBS_NB_LOCAL_CACHE_MISS,IBS_LOAD_DC_L2_HIT
432,"if (alpha) *dst = ScaleColor(pixss[To1D(u, v, i, s.frameSize - 1)], alpha) + ScaleColor(*dst, 255 - alpha);",2562,1320,184810,157,157,,1242,534,413,,6,,413,744
```

Splitting either side of the addition into their own for-loops shows that:
```
Line,Source,IBS_LOAD_STORE,IBS_LOAD,IBS_DC_MISS_LAT,IBS_NB_CACHE_MODIFIED,IBS_NB_LOCAL_CACHE_MODIFIED,IBS_NB_REMOTE_CACHE_MODIFIED,IBS_STORE,IBS_STORE_DC_MISS,IBS_NB_LOCAL_DRAM,IBS_NB_REMOTE_DRAM,IBS_NB_LOCAL_CACHE_OWNED,IBS_NB_REMOTE_CACHE_OWNED,IBS_NB_LOCAL_CACHE_MISS,IBS_LOAD_DC_L2_HIT
434,"*dst = ScaleColor(*dst, 255 - alpha);",941,485,104187,56,56,,456,242,228,,6,,228,195
452,"*dst += ScaleColor(pixss[To1D(u, v, i, s.frameSize - 1)], alpha);",450,450,41502,197,197,,450,290,62,,,,62,191
```
Doing so also messes up the colors when two tanks overlap.
At this point I can only reason that it is a long data-dependency on pixs through p0ss, p1ss, p2ss, p3ss  interpol weights?

Using the scaled pixels code we pragmad out (without really optimizing for data access other than the outer loop)  yields FPS 30 and average frame time 21ms
Moving it to SoA worsens performance tho to like 20 FPS?


We have a decent SoA that can play niceish with the GPGPU. I will start GPGPU.

#### 2024/06/17

The use of `Surface**` last_targets doesn't port well to GPGPU.
The `Surface*` is only used for `->pixels` (uint*), `->height` (int, basically a constant), and `->width` (int, also basically a constant).
We can add `defines` and rely on `uint*` to play nicer with the GPGPU.

The next issue is that s.scaledPixels is also a `uint**`.
Likewise, `**` doesn't port to OpenCL, so the structure needs to be flattened.
Which is just blocking me.
My approach is now to break it into smaller kernels that are collected host side.
The host will become the bottle-neck.
But I'll cross that bridge when I get there.


## Final performance