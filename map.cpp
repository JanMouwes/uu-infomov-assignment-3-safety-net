#include "precomp.h"

#include "game.h"

// Fast map rendering code by Conor Holden

Map::Map()
{
	// load color map
	bitmap = new Surface( "assets/colours.png" );
	width = bitmap->width;
	height = bitmap->height;
	// load height map
	Surface heightMap( "assets/heightmap.png" );
	elevation = new int[width * height];
	for (int i = 0; i < width * height; i++) elevation[i] = heightMap.pixels[i] & 255;
	// set intial focus to centre of map
	focus = make_int2( width >> 1, height >> 1 );
	// all done; original maps will be deleted when leaving scope.
}

void Map::Init(Kernel* draw_map)
{
	this->draw_map = draw_map;
	this->draw_map_bitmap_buffer = new Buffer(bitmap->height * bitmap->width, bitmap->pixels);
	//this->draw_map_bitmap_buffer->CopyFromDevice();
	this->draw_map_surface_buffer = new Buffer(SCRWIDTH * SCRHEIGHT);
}

float inv100 = 1.0f / 100.0f;
const float inv_SCRWIDTH = 1.0f / SCRWIDTH;
const float inv_SCRHEIGHT = 1.0f / SCRHEIGHT;
const float aspectRatio = (float)SCRHEIGHT / (float)SCRWIDTH;

Surface last_frame( SCRWIDTH, SCRHEIGHT );

void Map::UpdateView( Surface* target, float scale )
{
	// determine what map square to draw: centered at location 'focus', clamped to the edges of the map
	const int mapSliceSizeX = (int)((width * scale) * inv100);
	const int mapSliceSizeY = (int)((float)mapSliceSizeX * aspectRatio);
	int mapX1 = max( 0, focus.x - (mapSliceSizeX >> 1) );
	int mapY1 = max( 0, focus.y - (mapSliceSizeY >> 1) );
	int mapX2 = mapX1 + mapSliceSizeX;
	int mapY2 = mapY1 + mapSliceSizeY;
	if (mapX2 >= (width - 1))
	{
		int shift = mapX2 - (width - 2);
		mapX1 = max( 0, mapX1 - shift );
		mapX2 -= shift;
	}
	if (mapY2 >= (height - 1))
	{
		int shift = mapY2 - (height - 2);
		mapY1 = max( 0, mapY1 - shift );
		mapY2 -= shift;
	}
	focus.x = (mapX1 + mapX2) >> 1;
	focus.y = (mapY1 + mapY2) >> 1;
	view.x = mapX1, view.y = mapY1;
	view.z = mapX2, view.w = mapY2;
}

void Map::Draw( Surface* target )
{
	const uint dx = ((view.z - view.x) * 16384) * inv_SCRWIDTH;
	const uint dy = ((view.w - view.y) * 16384) * inv_SCRHEIGHT;



	draw_map->SetArguments(
		Game::screen_buffer,
		draw_map_bitmap_buffer,
		bitmap->width,
		dx,
		dy,
		view.x,
		view.y
	);

	draw_map->Run2D(make_int2( SCRWIDTH, SCRHEIGHT));

	// draw pixels

#if 0
	for (int y = 0; y < SCRHEIGHT; y++)
	{
		const uint y_fp = (view.y << 14) + y * dy;
		const uint* mapLine = bitmap->pixels + (y_fp >> 14) * width;
		uint* dst = target->pixels + y * SCRWIDTH;
		uint* lst = last_frame.pixels + y * SCRWIDTH;

		// bottom 14 bits
		constexpr uint least_significant_14 = 16383;
		const uint y_frac = y_fp & least_significant_14;

		// fixed-point: 0/18/14
		uint x_fp = view.x << 14;
		for (int x = 0; x < SCRWIDTH; x++, x_fp += dx)
		{
			const uint mapPos = x_fp >> 14;
			const uint p1 = mapLine[mapPos];
			const uint p2 = mapLine[mapPos + 1]; // mem
			uint combined = p1 + p2; // int
			const uint p3 = mapLine[mapPos + width]; // mem
			combined += p3; //int
			const uint p4 = mapLine[mapPos + width + 1]; // mem
			combined += p4; //int
			if (*lst != combined)
			{
				const uint x_frac = x_fp & least_significant_14; // integer
				*lst = combined; // memory
				const uint w1 = ((least_significant_14 - x_frac) * (least_significant_14 - y_frac)) >> 20; // integer
				const uint w3 = ((least_significant_14 - x_frac) * y_frac) >> 20;
				const uint w2 = (x_frac * (least_significant_14 - y_frac)) >> 20;
				const uint w4 = 255 - (w1 + w2 + w3);
				*dst = ScaleColor( p1, w1 ) + ScaleColor( p2, w2 ) + ScaleColor( p3, w3 ) + ScaleColor( p4, w4 ); //
			}
			dst++;
			lst++;
		}
	}
#endif
}

/**
 * playground for writing the kernel
 */
void do_map_draw(uint x_id, uint y_id,
                 uint* out_surface,
                 uint* in_bitmap,
                 uint in_bitmap_width,
                 uint dx, uint dy,
                 uint view_x, uint view_y)
{
	// int x_id = get_global_id(0);
	// int y_id = get_global_id(1);

	const uint y_fp = ((view_y << 14) + y_id * dy);
	const uint y_frac = y_fp & 16383;
	const uint x_fp = (view_x << 14);

	const uint* map_line = in_bitmap + (y_fp >> 14) * in_bitmap_width;

	const uint mapPos = x_fp >> 14;
	const uint p1 = map_line[mapPos];
	const uint p2 = map_line[mapPos + 1]; // mem
	const uint p3 = map_line[mapPos + in_bitmap_width]; // mem
	const uint p4 = map_line[mapPos + in_bitmap_width + 1]; // mem

	// const bool has_colour_changed = *lst != combined;
	// if (has_colour_changed)
	// {
		const uint x_frac = x_fp & 16383; // integer
		// *lst = combined; // memory
		const uint w1 = ((16383 - x_frac) * (16383 - y_frac)) >> 20; // integer
		const uint w3 = ((16383 - x_frac) * y_frac) >> 20;
		const uint w2 = (x_frac * (16383 - y_frac)) >> 20;
		const uint w4 = 255 - (w1 + w2 + w3);

		const uint out_1 = ((((p1 & 0xff00ff) * w1) >> 8) & 0x00ff00ff) + ((((p1 & 0xff00ff00) >> 8) * w1) & 0xff00ff00);
		const uint out_2 = ((((p2 & 0xff00ff) * w2) >> 8) & 0x00ff00ff) + ((((p2 & 0xff00ff00) >> 8) * w2) & 0xff00ff00);
		const uint out_3 = ((((p3 & 0xff00ff) * w3) >> 8) & 0x00ff00ff) + ((((p3 & 0xff00ff00) >> 8) * w3) & 0xff00ff00);
		const uint out_4 = ((((p4 & 0xff00ff) * w4) >> 8) & 0x00ff00ff) + ((((p4 & 0xff00ff00) >> 8) * w4) & 0xff00ff00);
		const uint out = out_1 + out_2 + out_3 + out_4;

		// *dst = out;
		out_surface[y_id * SCRWIDTH + x_id] = out;
	// }
	// lst++;
}


int2 Map::ScreenToMap( int2 pos )
{
	float u = (float)pos.x * inv_SCRWIDTH;
	float v = (float)pos.y * inv_SCRHEIGHT;
	return make_int2( view.x + (int)(u * (view.z - view.x)), view.y + (int)(v * (view.w - view.y)) );
}
