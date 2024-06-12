#pragma once

namespace Tmpl8
{

#define GRIDSIZE		64
#define CELLCAPACITY	256

struct ActorList 
{ 
	uint tank[CELLCAPACITY];
	int count = 0; 
};

class Grid
{
public:
	Grid() = default;
	void Clear();
	void Populate(const vector<SpatialComponent>& tanks);
	ActorList& FindNearbyTanks( float2 position, uint tank_id = 0 );
private:
	ActorList cell[GRIDSIZE * GRIDSIZE];
	ActorList answer; // we'll use this to return a list of nearby actors
};

} // namespace Tmpl8