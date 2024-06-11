#pragma once
#include <unordered_map>
#include <array>

#include "entity.h"
#include "EntityManager.h"
#include "containers/DynamicArray.h"

class SpatialManager
{
    // Handle to a component instance.
    struct Instance
    {
        entity_id index;
    };
    
    // Create an instance from an index to the data arrays.
    static Instance make_instance(entity_id i)
    {
        Instance inst = {i};
        return inst;
    }

    struct InstanceData
    {
        float2 pos;
        float2 dir;
        float2 velocity;
    };
    
    // map_ adds a level of indirection that allows SpatialManager to store entities sparsely. Indexing map_[entity_id]
    // yields the index of the component, or -1 if the component doesn't exist
    Templ8::DynamicArray<entity_id> map_;
    vector<InstanceData> data_;

    // Returns the component instance for the specified entity or a nil instance
    // if the entity doesn't have the component.
    Instance Lookup(entity_id e)
    {
        return make_instance(map_[e]);
    }

    float2 Pos(Instance instance) const { return data_[instance.index].pos; }
    float2 Dir(Instance instance) const { return data_[instance.index].dir; }

    void Simulate()
    {
        for (entity_id i = 0; i < data_.size(); ++i)
        {
            data_[i].pos += data_[i].velocity;
        }
    }
};
