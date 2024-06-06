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

## Final performance