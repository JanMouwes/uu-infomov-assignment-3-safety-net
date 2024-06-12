#pragma once

using entity = uint64_t;
// MAX_ENTITIES cannot be larger than 2^(64)-1
// Current value (8192) is arbitrary.
#define MAX_ENTITIES 8192 * 2

namespace Templ8
{
    // Entity
    static entity NEXT_ENTITY = 0;

    static entity NewEntity()
    {
        assert(NEXT_ENTITY < MAX_ENTITIES);
        return NEXT_ENTITY++;
    }
}
