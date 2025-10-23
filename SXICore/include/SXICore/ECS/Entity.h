#pragma once

#include "../MPL/Macros.h"

namespace sxi::ecs
{
    template <typename TArchetype>
    SXI_MPL_STRONG_TYPEDEF(size_t, EntityIndex);

    namespace detail
    {
        template <typename TSettings, typename TArchetype>
	    class ArchetypeStorage;

        template <typename TArchetype>
        SXI_MPL_STRONG_TYPEDEF(size_t, EntityHandleDataIndex);

        template <typename TArchetype>
        struct Entity final
        {
            EntityHandleDataIndex<TArchetype> handleDataIndex;
            bool alive;
        };

        template <typename TArchetype>
        class EntityHandleData final
        {
            EntityIndex<TArchetype> index;
            int counter;

            template <typename T, typename U>
            friend class ArchetypeStorage;
        };
    }

    template <typename TArchetype>
    class EntityHandle final
    {
        detail::EntityHandleDataIndex<TArchetype> handleDataIndex;
        int counter;

            template <typename T, typename U>
            friend class detail::ArchetypeStorage;
    };
}