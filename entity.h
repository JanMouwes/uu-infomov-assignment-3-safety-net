#pragma once

using entity = uint64_t;
// MAX_ENTITIES cannot be larger than 2^(64)-1
// Current value (8192) is arbitrary.
#define MAX_ENTITIES 8192

namespace Templ8
{
    // Entity
    static inline entity NEXT_ENTITY = 0;

    static entity NewEntity()
    {
        assert(NEXT_ENTITY < MAX_ENTITIES);
        return NEXT_ENTITY++;
    }
}
