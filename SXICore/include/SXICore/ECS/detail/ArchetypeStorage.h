#pragma once

#include "../../MPL/Filter.h"
#include "../../MPL/Rename.h"
#include "../../MPL/IsSubset.h"
#include "../../MPL/TypeListOperations.h"
#include <assert.h>
#include <iostream>

namespace sxi::ecs::detail
{
    template <typename TSettings, typename TArchetype>
	class ArchetypeStorage final 
	{
		size_t size{};
		size_t newSize{};
		size_t capacity{};

		using ArchetypeComponents = mpl::Filter<TSettings::template IsComponentFilter, TArchetype>;

		template <typename... Ts>
    	using TupleOfVectors = std::tuple<std::vector<Ts>...>;
		sxi::mpl::Rename<TupleOfVectors, ArchetypeComponents> components;

		std::vector<Entity<TArchetype>> entities;
		std::vector<EntityHandleData<TArchetype>> handleDatas;

		void reserve(size_t newCapacity)
		{
			assert(newCapacity > capacity);
			
			entities.resize(newCapacity);
			sxi::mpl::forTuple([newCapacity](auto& c){
				c.resize(newCapacity);
			}, components);

			for (size_t i = capacity; i < newCapacity; ++i)
				entities[i].alive = false;

			capacity = newCapacity;
		}

		void reserveIfNeeded()
		{
			if (capacity > newSize)
				return;

			reserve(2 * capacity);
		}

        [[nodiscard]] Entity<TArchetype>& entity(EntityIndex<TArchetype> index) noexcept
        {
            return entities[index];
        }

        [[nodiscard]] const Entity<TArchetype>& entity(EntityIndex<TArchetype> index) const noexcept
        {
            return entities[index];
        }

        [[nodiscard]] EntityHandleData<TArchetype>& entityHandleData(const EntityHandle<TArchetype>& handle) noexcept
        {
            return handleDatas[handle.handleDataIndex];
        }

        [[nodiscard]] const EntityHandleData<TArchetype>& entityHandleData(const EntityHandle<TArchetype>& handle) const noexcept
        {
            return handleDatas[handle.handleDataIndex];
        }

        [[nodiscard]] EntityHandleData<TArchetype>& entityHandleData(EntityIndex<TArchetype> index) noexcept
        {
            return handleDatas[entity(index).handleDataIndex];
        }

        [[nodiscard]] const EntityHandleData<TArchetype>& entityHandleData(EntityIndex<TArchetype> index) const noexcept
        {
            return handleDatas[entity(index).handleDataIndex];
        }

		void invalidateHandle(EntityIndex<TArchetype> index) noexcept
		{
			++entityHandleData(index).counter;
		}

		void refreshHandle(EntityIndex<TArchetype> index) noexcept
		{
			entityHandleData(index).index = index;
		}

		[[nodiscard]] size_t refreshImpl() noexcept
		{
			EntityIndex<TArchetype> left{0}, right{newSize - 1};
            while (true)
            {
                // go from left
                while (true)
                {
                    if (left > right)
                        return left;
                    if (!entities[left].alive)
                        break;
                    ++left;
                }
                // go from right
                while (true)
                {
                    if (left >= right)
                        return left;
                    if (entities[right].alive)
                        break;
					invalidateHandle(right);
                    --right;
                }

                assert(!entities[left].alive);
                assert(entities[right].alive);

                std::swap(entities[left], entities[right]);
				
				refreshHandle(left);
				invalidateHandle(right);
				refreshHandle(right);

				sxi::mpl::forTuple([left, right](auto& c){
					std::swap(c[left], c[right]);
				}, components);
				++left;
				--right;
            }
            return right;
		}

        template <typename... Ts>
        struct ExpandCallHelper
        {
            template <typename Func>
            static void call(EntityIndex<TArchetype> index, ArchetypeStorage<TSettings, TArchetype>& as, Func&& func)
            {
                func(index, as.template component<Ts>(index)...);
            }
        };

	public:
		ArchetypeStorage(size_t initialCapacity=1)
		{
			assert(initialCapacity > 0);

			reserve(initialCapacity);
		}

		[[nodiscard]] EntityIndex<TArchetype> createEntity()
		{
			reserveIfNeeded();
			
			EntityIndex<TArchetype> freeIndex{newSize++};

			Entity<TArchetype>& e = entities[freeIndex];
			assert(!e.alive);
			e.handleDataIndex = std::numeric_limits<size_t>::max();
			e.alive = true;

			return freeIndex;
		}

		[[nodiscard]] EntityHandle<TArchetype> createHandle(EntityIndex<TArchetype> index)
		{
			EntityHandleDataIndex<TArchetype> freeIndex{handleDatas.size()};
			entity(index).handleDataIndex = freeIndex;
            EntityHandleData<TArchetype> handleData;
            handleData.index = index;
            handleData.counter = 0;
			handleDatas.push_back(handleData);
            EntityHandle<TArchetype> handle;
            handle.handleDataIndex = freeIndex;
            handle.counter = 0;
			return handle;
		}

		[[nodiscard]] bool isEntityHandleValid(const EntityHandle<TArchetype>& handle) const noexcept
		{
			return entityHandleData(handle).counter == handle.counter;
		}

		template <typename TComponent>
		[[nodiscard]] TComponent& component(EntityIndex<TArchetype> index) noexcept
		{
			return std::get<std::vector<TComponent>>(components)[index];
		}

		template <typename TComponent>
		[[nodiscard]] const TComponent& component(EntityIndex<TArchetype> index) const noexcept
		{
			return std::get<std::vector<TComponent>>(components)[index];
		}

		bool isAlive(EntityIndex<TArchetype> index) const noexcept
		{
			return entity(index).alive;
		}

		void kill(EntityIndex<TArchetype> index) noexcept
		{
			entity(index).alive = false;
		}

		bool isAlive(const EntityHandle<TArchetype>& handle) const noexcept
		{
			return entity(entityHandleData(handle).index).alive;
		}

		void kill(const EntityHandle<TArchetype>& handle) noexcept
		{
			entity(entityHandleData(handle).index).alive = false;
		}

		void refresh() noexcept
		{
			if (newSize == 0)
			{
				size = 0;
				return;
			}

			size = newSize = refreshImpl();
		}
        
        template <typename Func>
        void forEntities(Func&& func)
        {
            for (EntityIndex<TArchetype> i{0}; i < size; ++i)
				func(i);
        }
        
        template <typename TSignature, typename Func>
        void forComponents(Func&& func)
        {	
			if constexpr(mpl::IsSubset<TArchetype, TSignature>::value)
			{	
				using RequiredComponents = mpl::Filter<TSettings::template IsComponentFilter, TSignature>;
				using Helper = mpl::Rename<ExpandCallHelper, RequiredComponents>;
				for (EntityIndex<TArchetype> i{0}; i < size; ++i)
					Helper::call(i, *this, func);
			}
        }
	};
}