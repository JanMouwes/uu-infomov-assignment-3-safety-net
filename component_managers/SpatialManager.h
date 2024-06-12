#pragma once

#include "entity.h"
#include "EntityManager.h"
#include "containers/DynamicArray.h"

// Handle to a component instance.
struct SpatialInstance
{
    int index;
};

// TODO: Rename to SpatialComponent when it no longer clashes.
struct SpatialInstanceData
{
    float2 pos;
    float2 dir;
    float2 velocity;
};

/*
 * A Component manager implemented based on: https://bitsquid.blogspot.com/2014/09/building-data-oriented-entity-system.html.
 */
class SpatialManager
{
public:
    void Init()
    {
        data_.Reserve(MAX_ENTITIES);
    }

    /*
     * MakeInstance creates a new SpatialInstance given an index into SpatialManager's data_ array.
     */
    SpatialInstance MakeInstance(int index);
    {
        SpatialInstance inst = {index};
        return inst;
    }

    /*
     * Register registers a new entity with this SpatialManager. The entity's initial spatial data is set to d.
     */
    SpatialInstance Register(entity_id e, SpatialInstanceData d)
    {
        map_.PutUnique(e);
        data_.Push(d);
        SpatialInstance instance = Lookup(e);
        return instance;
    }

    // TODO: Destroying entities. See: https://bitsquid.blogspot.com/2014/09/building-data-oriented-entity-system.html.

    /*
     * Lookup returns a SpatialInstance into this SpatialManager's data array. If the entity is not registered with this
     * SpatialManager, it returns a SpatialInstance indexing into -1.
     */
    SpatialInstance Lookup(entity_id e)
    {
        unsigned int index;
        if (!map_.Exists(e, index))
            return MakeInstance(-1);
        return MakeInstance(index);
    }

    /*
     * Exists returns true if the given entity is registered with this SpatialManager, otherwise it returns false.
     */
    bool Exists(entity_id e)
    {
        SpatialInstance instance = Lookup(e);
        return instance.index != -1;
    }

    float2 Pos(SpatialInstance instance) { return data_[instance.index].pos; }
    float2 Dir(SpatialInstance instance) { return data_[instance.index].dir; }

    /*
     * Tick ticks the spatial component for all entities registered with this SpatialManager
     */
    void Tick()
    {
        for (entity_id i = 0; i < data_.GetNum(); ++i)
        {
            data_[i].pos += data_[i].velocity;
        }
    }

private:
    /*
     * map_ adds a level of indirection to SpatialManager that allows it to store entities sparsely. The entity 'e'
     * has index 'i' in 'map_' iff the data stored at index 'i' in 'data_' belongs to 'e'.
     */
    Templ8::SortedList<entity_id, MAX_ENTITIES> map_;
    Templ8::DynamicArray<SpatialInstanceData> data_;
};
