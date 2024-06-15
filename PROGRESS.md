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

### SpriteInstance::Draw

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


## Final performance