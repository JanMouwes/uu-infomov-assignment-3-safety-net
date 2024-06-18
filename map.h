#pragma once

namespace Tmpl8
{

class Map
{
public:
	Map();

	void Init(Kernel* draw_map);
	void UpdateView( Surface* target, float scale );
	void Draw( Surface* target );
	void SetFocus( int2 pos ) { focus = pos; }
	void MoveFocus( int2 delta ) { focus += delta; }
	int2 GetFocus() const { return focus; }
	int2 MapSize() { return make_int2( width, height ); }
	int2 ScreenToMap( int2 pos );
	static inline Surface* bitmap = 0;
	int2 focus;
	int* elevation;
	int width, height;
	int4 view; // visible portion of the map
	Kernel* draw_map;
	Buffer* draw_map_bitmap_buffer;
	Buffer* draw_map_surface_buffer;
};

} // namespace Tmpl8