#pragma once

#include "entity.h"
#include "EntityManager.h"
#include "containers/DynamicArray.h"

// Handle to a component instance.
struct SpatialInstance
{
    unsigned int index;
};

struct SpatialInstanceData
{
    float2 pos;
    float2 dir;
    float2 velocity;
};

class SpatialManager
{
public:
    void Init()
    {
        data_.Reserve(MAX_ENTITIES);
    }

    // Create an instance from an index to the data arrays.
    SpatialInstance MakeInstance(unsigned int index)
    {
        SpatialInstance inst = {index};
        return inst;
    }

    SpatialInstance Register(entity_id e, SpatialInstanceData d)
    {
        map_.PutUnique(e);
        data_.Push(d);
        SpatialInstance instance = Lookup(e);
        return instance;
    }


    // Returns the component instance for the specified entity or a nil instance
    // if the entity doesn't have the component.
    SpatialInstance Lookup(entity_id e)
    {
        unsigned int index;
        if (!map_.Exists(e, index))
            return MakeInstance(-1);
        return MakeInstance(index);
    }

    float2 Pos(SpatialInstance instance) { return data_[instance.index].pos; }
    float2 Dir(SpatialInstance instance) { return data_[instance.index].dir; }

    void Simulate()
    {
        for (entity_id i = 0; i < data_.GetNum(); ++i)
        {
            data_[i].pos += data_[i].velocity;
        }
    }

private:
    // map_ adds a level of indirection that allows SpatialManager to store entities sparsely. Indexing map_[entity_id]
    // yields the index of the component, or -1 if the component doesn't exist
    Templ8::SortedList<entity_id, MAX_ENTITIES> map_;
    Templ8::DynamicArray<SpatialInstanceData> data_;
};
