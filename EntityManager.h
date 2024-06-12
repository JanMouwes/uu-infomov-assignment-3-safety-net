
#include "entity.h"
#include "containers/SortedList.h"

// int is used because Agner Fox's containers use int indices.
using entity_id = int;

class EntityManager
{
public:
    /*
     * Create creates a new entity. It fails if the max number of entities exists.
     */
    entity_id Create()
    {
        ++_next;
        assert(_next < MAX_ENTITIES);
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
private:
    Templ8::SortedList<entity_id, MAX_ENTITIES> _entities;
    entity_id _next;
};