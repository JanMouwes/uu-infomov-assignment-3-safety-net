#include "template/common.h"

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
    int sprite_frame_size,
    global int* bounding_boxes,
    global int* last_poss
)
{
    int idx = get_global_id(0);
    float x = poss[2 * idx + 0];
    float y = poss[2 * idx + 1];
    
    // printf("float2 pos: %f, %f\n", x, y);
    
    // int2 int_pos = make_int2(x, y)
    int2 int_pos = { (int)x, (int)y };
    
    // printf("int2 pos: %d, %d\n", int_pos.x, int_pos.y);
    
    int x1 = int_pos.x - sprite_frame_size / 2;
    bounding_boxes[4 * idx + 0] = x1;
    
    int x2 = x1 + sprite_frame_size;
    bounding_boxes[4 * idx + 1] = x2;
    
    int y1 = int_pos.y - sprite_frame_size / 2;
    bounding_boxes[4 * idx + 2] = y1;
    
    int y2 = y1 + sprite_frame_size;
    bounding_boxes[4 * idx + 3] = y2;
    
    // printf("%d, %d, %d, %d\n", x1, x2, y1, y2);
    
    if (x1 < 0 || y1 < 0 || x2 >= MAPWIDTH || y2 >= MAPHEIGHT) return;
    
    last_poss[2 * idx + 0] = x1;
    last_poss[2 * idx + 1] = y1;
}

