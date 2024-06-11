
#include "entity.h"
#include "containers/SortedList.h"

using entity_id = int;

class EntityManager
{
    Templ8::SortedList<entity_id, MAX_ENTITIES> _entities;
    entity_id _next;

public:
    entity_id Create()
    {
        ++_next;
        while (Alive(_next))
            ++_next;
        _entities.Put(_next);
        return _next;
    }

    bool Alive(entity_id e)
    {
        unsigned int index;
        return _entities.Exists(e, index);
    }

    void Destroy(entity_id e)
    {
        unsigned int index;
        if (!_entities.Exists(e, index)) return;
        _entities.Remove(index);
    }
};