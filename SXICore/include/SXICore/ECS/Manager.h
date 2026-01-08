#pragma once

#include "../MPL/TypeListOperations.h"
#include "../Types.h"
#include "Entity.h"
#include "detail/ArchetypeStorage.h"

#include <cstddef>

namespace sxi::ecs
{
using namespace types;

template<typename TSettings>
class Manager final
{
    using Settings = TSettings;
    using ArchetypeList = TSettings::ArchetypeList;

    template<typename... Ts>
    using TupleOfArchetypeStorages = std::tuple<detail::ArchetypeStorage<Settings, Ts>...>;
    sxi::mpl::Rename<TupleOfArchetypeStorages, ArchetypeList> archetypes;

    template<typename TArchetype>
    detail::ArchetypeStorage<TSettings, TArchetype>& archetypeStorage() noexcept
    {
        return std::get<detail::ArchetypeStorage<TSettings, TArchetype>>(archetypes);
    }

    template<typename TArchetype>
    const detail::ArchetypeStorage<TSettings, TArchetype>& archetypeStorage() const noexcept
    {
        return std::get<detail::ArchetypeStorage<TSettings, TArchetype>>(archetypes);
    }

public:
    template<typename TArchetype>
    EntityIndex<TArchetype> createEntity()
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        return archetypeStorage<TArchetype>().createEntity();
    }

    template<typename TComponent, typename TArchetype>
    TComponent& component(EntityIndex<TArchetype> index) noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");
        static_assert(Settings::template isComponent<TComponent>(), "TComponent must be a component");

        return archetypeStorage<TArchetype>().template component<TComponent>(index);
    }

    template<typename TArchetype>
    bool isAlive(EntityIndex<TArchetype> index) const noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        return archetypeStorage<TArchetype>().isAlive(index);
    }

    template<typename TArchetype>
    void kill(EntityIndex<TArchetype> index) noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        archetypeStorage<TArchetype>().kill(index);
    }

    template<typename TArchetype>
    [[nodiscard]] EntityHandle<TArchetype> createHandle(EntityIndex<TArchetype> index)
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        return archetypeStorage<TArchetype>().createHandle(index);
    }

    template<typename TComponent, typename TArchetype>
    TComponent& component(const EntityHandle<TArchetype>& handle) noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");
        static_assert(Settings::template isComponent<TComponent>(), "TComponent must be a component");

        return archetypeStorage<TArchetype>().template component<TComponent>(handle);
    }

    template<typename TArchetype>
    size_t entityCount() const noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        return archetypeStorage<TArchetype>().entityCount();
    }

    template<typename TArchetype>
    bool isAlive(const EntityHandle<TArchetype>& handle) const noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        return archetypeStorage<TArchetype>().isAlive(handle);
    }

    template<typename TArchetype>
    void kill(const EntityHandle<TArchetype>& handle) noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        archetypeStorage<TArchetype>().kill(handle);
    }

    template<typename TArchetype>
    [[nodiscard]] bool isEntityHandleValid(const EntityHandle<TArchetype>& handle) const noexcept
    {
        static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

        return archetypeStorage<TArchetype>().isEntityHandleValid(handle);
    }

    void refresh() noexcept
    {
        mpl::forTuple(
            [](auto& as)
            {
                as.refresh();
            },
            archetypes);
    }

    template<typename Func>
    void forEntities(Func&& func, u32 start, u32 end)
    {
        mpl::forTuple(
            [&func, start, end](auto& as)
            {
                as.forEntities(func, start, end);
            },
            archetypes);
    }

    template<typename TArchetype, typename Func>
    void forEntities(Func&& func, u32 start, u32 end)
    {
        archetypeStorage<TArchetype>().forEntities(func);
    }

    template<typename TSignature, typename Func>
    void forEntitiesMatching(Func&& func, u32 start, u32 end)
    {
        static_assert(Settings::template isSignature<TSignature>(), "TSignature is not a signature");

        mpl::forTuple(
            [&func, start, end](auto& as)
            {
                as.template forComponents<TSignature>(func, start, end);
            },
            archetypes);
    }
};
} // namespace sxi::ecs