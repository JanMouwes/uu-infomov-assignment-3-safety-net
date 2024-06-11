
#include <unordered_set>

using entity_id = uint32_t;

struct Entity
{
    entity_id id;
};

class EntityManager
{
    unordered_set<Entity> _entities;
    Entity _next;

public:
    Entity Create()
    {
        ++_next.id;
        while (Alive(_next))
            ++_next.id;
        _entities.insert(_next);
        return _next;
    }

    bool Alive(Entity e)
    {
        return _entities.count(e) > 0;
    }

    void Destroy(Entity e)
    {
        _entities.erase(e);
    }
};