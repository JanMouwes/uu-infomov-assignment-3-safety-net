#include "template/common.h"

inline uint ScaleColor( const uint c, const uint scale )
{
	const uint rb = (((c & 0xff00ff) * scale) >> 8) & 0x00ff00ff;
	const uint ag = (((c & 0xff00ff00) >> 8) * scale) & 0xff00ff00;
	return rb + ag;
}

inline uint To1D(uint x, uint y, uint z, uint square)
{
    return z * (square * square) + y * square + x;
}

__kernel void myKernel( write_only image2d_t outimg )
{
	// get thread id
	int x = get_global_id( 0 );
	int y = get_global_id( 1 );
	int id = x + SCRWIDTH * y;
	if (id >= (SCRWIDTH * SCRHEIGHT)) return;
	// do calculations
	float dx = (float)x / (float)SCRWIDTH - 0.5f;
	float dy = (float)y / (float)SCRHEIGHT - 0.5f;
	float sqdist = native_sqrt( dx * dx + dy * dy ); // sqrtf causes ptx error..
	// send result to output surface
	if (sqdist < 0.1f) write_imagef( outimg, (int2)(x, y), (float4)( 1, 1, 1, 1 ) );
}

__kernel void computeBoundingBoxes(
    global float* poss,
    const int sprite_frame_size,
    global int* bounding_boxes,
    global int* last_poss
)
{
    int idx = get_global_id(0);
    float x = poss[2 * idx + 0];
    float y = poss[2 * idx + 1];
    
    // int2 int_pos = make_int2(x, y)
    int2 int_pos = { (int)x, (int)y };
    
    int x1 = int_pos.x - sprite_frame_size / 2;
    bounding_boxes[4 * idx + 0] = x1;
    
    int x2 = x1 + sprite_frame_size;
    bounding_boxes[4 * idx + 1] = x2;
    
    int y1 = int_pos.y - sprite_frame_size / 2;
    bounding_boxes[4 * idx + 2] = y1;
    
    int y2 = y1 + sprite_frame_size;
    bounding_boxes[4 * idx + 3] = y2;
    
    if (x1 < 0 || y1 < 0 || x2 >= MAPWIDTH || y2 >= MAPHEIGHT) return;
    
    // Also just do the last_poss while we're at it
    // x1 and y1 is right here as well as the bounds check.
    // The last_surface setting in the bounds check is still necessary, because it bears on the behaviour
    // of the sprite remove.
    last_poss[2 * idx + 0] = x1;
    last_poss[2 * idx + 1] = y1;
}

__kernel void computeInterpolWeights(
    global float* poss,
    global uint* interpol_weights
)
{
    int idx = get_global_id(0);
    float x = poss[2 * idx + 0];
    float y = poss[2 * idx + 1];
    
    // TODO: This looks like some botched fixed-point arithmetic?
    // floor might give incorrect results used as is compared to floorf?
    //https://stackoverflow.com/questions/74488235/opencl-floor-function-doesnt-return-correct-number
    uint frac_xs = (int)(255.0f * (x - floor(x)));
    uint frac_ys = (int)(255.0f * (y - floor(y)));
    
    uint interpol_weight_0 = (frac_xs * frac_ys) >> 8;
    interpol_weights[idx * 4 + 0] = interpol_weight_0;
    // printf("%d placed as %d\n", interpol_weight_0, interpol_weights[idx * 4 + 0]);
    
    uint interpol_weight_1 = ((255 - frac_xs) * frac_ys) >> 8;
    interpol_weights[idx * 4 + 1] = interpol_weight_1;
    // printf("%d placed as %d\n", interpol_weight_1, interpol_weights[idx * 4 + 1]);
    
    uint interpol_weight_2 = (frac_xs * (255 - frac_ys)) >> 8;
    interpol_weights[idx * 4 + 2] = interpol_weight_2;
    // printf("%d placed as %d\n", interpol_weight_2, interpol_weights[idx * 4 + 2]);
    
    uint interpol_weight_3 = ((255 - frac_xs) * (255 - frac_ys)) >> 8;
    interpol_weights[idx * 4 + 3] = interpol_weight_3;
    // ("%d placed as %d\n", interpol_weight_3, interpol_weights[idx * 4 + 3]);
}

__kernel void computePixels(
    global uint* interpol_weights,
    const int sprite_frame_size,
    const int sprite_frame_count,
    const int sprite_stride,
    global int* frames,
    global uint* scaled_pixels,
    global uint* pixels
)
{
    int i = get_global_id(0);
    // if (i >= 2500) printf("%d", i);
    int frame = frames[i];
    
    uint scale1 = interpol_weights[i * 4 + 1];
    uint scale2 = interpol_weights[i * 4 + 2];
    uint scale3 = interpol_weights[i * 4 + 3];
    
    uint scale0 = interpol_weights[i * 4 + 0];
    // if (i == 1250) printf("scale0: %d\n", scale0);
    for (int v = 0; v < sprite_frame_size - 1; v++)
    {
        const uint row_origin = frame * sprite_frame_size + v * sprite_stride;
        for (int u = 0; u < sprite_frame_size - 1; u++)
        {
            uint scaled_pixel_index = scale0 * (sprite_frame_count * sprite_frame_size * sprite_frame_size) + row_origin + u;
            // if (i == 1250) printf("\tscaled_pixel_index: %d\n", scaled_pixel_index);
            // if (scaled_pixel_index >= 256 * 256 * 10 * ) printf("%d is doing something bad by accessing %d * (%d * %d *%d) + %d + u = %d\n", i, scale0, sprite_frame_count, sprite_frame_size, sprite_frame_size, row_origin, u, scaled_pixel_index);
            uint flatsrc0 = scaled_pixels[scaled_pixel_index];
            pixels[To1D(u, v, i, sprite_frame_size - 1)] = flatsrc0;
        }
    }
}