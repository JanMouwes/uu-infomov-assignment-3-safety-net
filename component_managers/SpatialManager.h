#pragma once
#include <unordered_map>
#include <array>

#include "entity.h"
#include "EntityManager.h"

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
    
    // map_ adds a level of indirection that allows SpatialManager to store entities sparsely.
    unordered_map<Entity, entity_id> map_;
    vector<InstanceData> data_;

    // Returns the component instance for the specified entity or a nil instance
    // if the entity doesn't have the component.
    Instance lookup(Entity e) const
    {
        if (map_.count(e) == 0) return make_instance(0);
        return make_instance(map_.find(e)->first.id);
    }

    float2 pos(Instance instance) const { return data_[instance.index].pos; }
    float2 dir(Instance instance) const { return data_[instance.index].dir; }

    void simulate()
    {
        for (entity_id i = 0; i < data_.size(); ++i)
        {
            data_[i].pos += data_[i].velocity;
        }
    }
    
};
