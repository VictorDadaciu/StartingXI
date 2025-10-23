#pragma once

#include "Settings.h"
#include "Entity.h"
#include "detail/ArchetypeStorage.h"
#include <iostream>

#include "../MPL/TypeListOperations.h"
#include "../MPL/IsSubset.h"

namespace sxi::ecs
{
    template <typename TSettings>
    class Manager final
    {
        using Settings = TSettings;
        using ArchetypeList = TSettings::ArchetypeList;

        template <typename... Ts>
        using TupleOfArchetypeStorages = std::tuple<detail::ArchetypeStorage<Settings, Ts>...>;
        sxi::mpl::Rename<TupleOfArchetypeStorages, ArchetypeList> archetypes;

        template <typename TArchetype>
        detail::ArchetypeStorage<TSettings, TArchetype>& archetypeStorage() noexcept
        {
            return std::get<detail::ArchetypeStorage<TSettings, TArchetype>>(archetypes);
        }

        template <typename TArchetype>
        const detail::ArchetypeStorage<TSettings, TArchetype>& archetypeStorage() const noexcept
        {
            return std::get<detail::ArchetypeStorage<TSettings, TArchetype>>(archetypes);
        }

    public:
        template <typename TArchetype>
        EntityIndex<TArchetype> createEntity()
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            return archetypeStorage<TArchetype>().createEntity();
        }

        template <typename TComponent, typename TArchetype>
        TComponent& component(EntityIndex<TArchetype> index) noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");
            static_assert(Settings::template isComponent<TComponent>(), "TComponent must be a component");

            return archetypeStorage<TArchetype>().template component<TComponent>(index);
        }

        template <typename TArchetype>
        bool isAlive(EntityIndex<TArchetype> index) const noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            return archetypeStorage<TArchetype>().isAlive(index);
        }

        template <typename TArchetype>
        void kill(EntityIndex<TArchetype> index) noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            archetypeStorage<TArchetype>().kill(index);
        }

        template <typename TArchetype>
        [[nodiscard]] EntityHandle<TArchetype> createHandle(EntityIndex<TArchetype> index)
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            return archetypeStorage<TArchetype>().createHandle(index);
        }

        template <typename TComponent, typename TArchetype>
        TComponent& component(const EntityHandle<TArchetype>& handle) noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");
            static_assert(Settings::template isComponent<TComponent>(), "TComponent must be a component");

            return archetypeStorage<TArchetype>().template component<TComponent>(handle);
        }

        template <typename TArchetype>
        bool isAlive(const EntityHandle<TArchetype>& handle) const noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            return archetypeStorage<TArchetype>().isAlive(handle);
        }

        template <typename TArchetype>
        void kill(const EntityHandle<TArchetype>& handle) noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            archetypeStorage<TArchetype>().kill(handle);
        }

        template <typename TArchetype>
        [[nodiscard]] bool isEntityHandleValid(const EntityHandle<TArchetype>& handle) const noexcept
        {
            static_assert(Settings::template isArchetype<TArchetype>(), "TArchetype must be an archetype");

            return archetypeStorage<TArchetype>().isEntityHandleValid(handle);
        }

        void refresh() noexcept
        {
            mpl::forTuple([](auto& as){
                as.refresh();
            }, archetypes);
        }
        
        template <typename Func>
        void forEntities(Func&& func)
        {
            mpl::forTuple([&func](auto& as){
                as.forEntities(func);
            }, archetypes);
        }
        
        template <typename TArchetype, typename Func>
        void forEntities(Func&& func)
        {
            archetypeStorage<TArchetype>().forEntities(func);
        }
        
        template <typename TSignature, typename Func>
        void forEntitiesMatching(Func&& func)
        {
            static_assert(Settings::template isSignature<TSignature>(), "TSignature is not a signature");

            mpl::forTuple([this, &func](auto& as){
                as.template forComponents<TSignature>(func);
            }, archetypes);
        }
    };
}