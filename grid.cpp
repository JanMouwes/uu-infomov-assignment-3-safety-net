#include "precomp.h"
#include "game.h"

void Grid::Clear()
{
	for( int i = 0; i < GRIDSIZE * GRIDSIZE; i++ ) cell[i].count = 0;
}

void Grid::Populate( const vector<SpatialComponent>& tanks )
{
	int2 mapSize = Game::map.MapSize();
	float2 posScale = GRIDSIZE * make_float2( 1.0f / mapSize.x, 1.0f / mapSize.y );
	for( uint s = tanks.size(), i = 0; i < s; i++ )
	{
		// calculate actor position in grid space
		int2 gridPos = make_int2( posScale * tanks[i].pos );
		// add actor to cell
		if (gridPos.x < 0 || gridPos.y < 0 || gridPos.x >= GRIDSIZE || gridPos.y >= GRIDSIZE) continue;
		ActorList& c = cell[gridPos.x + gridPos.y * GRIDSIZE];
		c.tank[c.count++ & (CELLCAPACITY - 1) /* better than overflow */] = i;
	}
}

ActorList& Grid::FindNearbyTanks(const float2 position, const uint tank_id )
{
	const int2 mapSize = Game::map.MapSize();
	const float2 posScale = GRIDSIZE * make_float2( 1.0f / mapSize.x, 1.0f / mapSize.y );
	const int2 gridPos = make_int2( posScale * position );
	const int2 topLeft( max( 0, gridPos.x - 1 ), max( 0, gridPos.y - 1 ) );
	const int2 bottomRight( min( GRIDSIZE - 1, gridPos.x + 1 ), min( GRIDSIZE - 1, gridPos.y + 1 ) );
	answer.count = 0;
	for (int x = topLeft.x; x <= bottomRight.x; x++)
	{
		for (int y = topLeft.y; y <= bottomRight.y; y++)
		{
			const uint cell_i = x + y * GRIDSIZE;
			for (int i = 0; i < cell[cell_i].count; i++)
			{
				const uint other_id = cell[cell_i].tank[i];
				if (other_id != tank_id) answer.tank[answer.count++ & (CELLCAPACITY - 1)] = other_id;
			}
		}
	}
	return answer;
}