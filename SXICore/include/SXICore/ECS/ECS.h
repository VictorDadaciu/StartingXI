#pragma once

#include <iostream>
#include <ostream>
#include <type_traits>
#include <bitset>
#include <vector>
#include <assert.h>

#include "../MPL/TypeList.h"
#include "../MPL/Contains.h"
#include "../MPL/Count.h"
#include "../MPL/IndexOf.h"
#include "../MPL/Repeat.h"
#include "../MPL/Tuple.h"
#include "../MPL/Filter.h"
#include "../MPL/TypeListOperations.h"
#include "../MPL/Macros.h"

namespace sxi::ecs
{
    template <
        typename TComponentList,
        typename TTagList,
        typename TSignatureList
    >
    struct Settings;

    SXI_MPL_STRONG_TYPEDEF(std::size_t, DataIndex);
    SXI_MPL_STRONG_TYPEDEF(std::size_t, EntityIndex);

    namespace detail
    {
        template <typename TSettings>
        struct Entity
        {
            using Settings = TSettings;
            using Bitset = Settings::Bitset;

            DataIndex dataIndex;

            Bitset bitset;

            bool alive;
        };

        template <typename TSettings>
        class ComponentStorage
        {
        private:
            using Settings = TSettings;
            using ComponentList = typename Settings::ComponentList;

            template <typename... Ts>
            using TupleOfVectors = std::tuple<std::vector<Ts>...>;
            mpl::Rename<TupleOfVectors, ComponentList> vectors;

        public:
            void resize(size_t newCapacity)
            {
                mpl::forTuple([this, newCapacity](auto& v) {
                    v.resize(newCapacity);
                }, vectors);
            }

            template <typename T>
            auto& component(DataIndex index) noexcept
            {
                return std::get<std::vector<T>>(vectors)[index];
            }
        };
    }
    
    template <typename... Ts> using Signature = mpl::typelist<Ts...>;
    template <typename... Ts> using SignatureList = mpl::typelist<Ts...>;
    
    template <typename... Ts> using ComponentList = mpl::typelist<Ts...>;
    template <typename... Ts> using TagList = mpl::typelist<Ts...>;

    namespace detail 
    {
        template <typename TSettings>
        struct SignatureBitsets;

        template <typename TSettings>
        struct SignatureBitsetsStorage;
    }

    template <
        typename TComponentList,
        typename TTagList,
        typename TSignatureList
    >
    struct Settings
    {
        using ComponentList = TComponentList;
        using TagList = TTagList;
        using SignatureList = TSignatureList;
        using TSettings = Settings<ComponentList, TagList, SignatureList>;

        using SignatureBitsets = typename detail::SignatureBitsets<TSettings>;
        using SignatureBitsetsStorage = typename detail::SignatureBitsetsStorage<TSettings>;

        template <typename T>
        static constexpr bool isComponent() noexcept
        {
            return mpl::Contains<T, ComponentList>::value;
        }

        template <typename T>
        static constexpr bool isTag() noexcept
        {
            return mpl::Contains<T, TagList>::value;
        }

        template <typename T>
        static constexpr bool isSignature() noexcept
        {
            return mpl::Contains<T, SignatureList>::value;
        }

        static constexpr size_t componentCount() noexcept
        {
            return mpl::Count<ComponentList>::value;
        }

        static constexpr size_t tagCount() noexcept
        {
            return mpl::Count<TagList>::value;
        }

        static constexpr size_t signatureCount() noexcept
        {
            return mpl::Count<SignatureList>::value;
        }

        template <typename T>
        static constexpr size_t componentId() noexcept
        {
            return mpl::IndexOf<T, ComponentList>::value;
        }

        template <typename T>
        static constexpr size_t tagId() noexcept
        {
            return mpl::IndexOf<T, TagList>::value;
        }

        template <typename T>
        static constexpr size_t signatureId() noexcept
        {
            return mpl::IndexOf<T, SignatureList>::value;
        }

        using Bitset = std::bitset<componentCount() + tagCount()>;

        template <typename T>
        static constexpr size_t componentBit() noexcept
        {
            return componentId<T>();
        }

        template <typename T>
        static constexpr size_t tagBit() noexcept
        {
            return componentCount() + tagId<T>();
        }
    };

    namespace detail
    {
        template <typename TSettings>
        struct SignatureBitsets
        {
            using TSignatureBitsets = SignatureBitsets<TSettings>;
            using SignatureList = typename TSettings::SignatureList;
            using Bitset = typename TSettings::Bitset;

            using BitsetRepeatedList = mpl::Repeat<TSettings::signatureCount(), Bitset>;
            using BitsetStorage = mpl::Tuple<BitsetRepeatedList>;

            template <typename T>
            using IsComponentFilter = std::bool_constant<TSettings::template isComponent<T>()>;
            template <typename T>
            using IsTagFilter = std::bool_constant<TSettings::template isTag<T>()>;

            template <typename TSignature>
            using SignatureComponents = mpl::Filter<IsComponentFilter, TSignature>;
            template <typename TSignature>
            using SignatureTags = mpl::Filter<IsTagFilter, TSignature>;
        };

        template <typename TSettings>
        class SignatureBitsetsStorage
        {
        private:
            using Settings = TSettings;
            using SignatureBitsets = typename Settings::SignatureBitsets;
            using SignatureList = typename SignatureBitsets::SignatureList;
            using BitsetStorage = typename SignatureBitsets::BitsetStorage;

            BitsetStorage storage;

        public:
            template <typename T>
            auto& signatureBitset() noexcept
            {
                static_assert(Settings::template isSignature<T>(), "Type parameter must be a valid signature");

                return std::get<Settings::template signatureId<T>()>(storage);
            }

        private:
            template <typename T>
            void initializeBitset() noexcept
            {
                auto& b(signatureBitset<T>());

                using SignatureComponents = typename SignatureBitsets::template SignatureComponents<T>;
                using SignatureTags = typename SignatureBitsets::template SignatureTags<T>;

                mpl::forTypes<SignatureComponents>([this, &b](auto t) {
                    b[Settings::template componentBit<SXI_MPL_TYPE(t)>()] = true;
                });
                mpl::forTypes<SignatureTags>([this, &b](auto t) {
                    b[Settings::template tagBit<SXI_MPL_TYPE(t)>()] = true;
                });
            }

        public:
            SignatureBitsetsStorage() noexcept
            {
                mpl::forTypes<SignatureList>([this](auto t) {
                    this->initializeBitset<SXI_MPL_TYPE(t)>();
                });
            }
        };
    }

    template <typename TSettings>
    class Manager
    {
    private:
        using Settings = TSettings;
        using TManager = Manager<Settings>;
        using Bitset = typename Settings::Bitset;
        using Entity = detail::Entity<Settings>;
        using SignatureBitsetsStorage = detail::SignatureBitsetsStorage<Settings>;
        using ComponentsStorage = detail::ComponentStorage<Settings>;

        size_t capacity{};
        size_t size{};
        size_t newSize{};

        std::vector<Entity> entities{};
        SignatureBitsetsStorage signatureBitsets{};
        ComponentsStorage components{};

        void resize(size_t newCapacity)
        {
            assert(capacity < newCapacity);

            entities.resize(newCapacity);
            components.resize(newCapacity);

            for (size_t i = capacity; i < newCapacity; ++i)
            {
                Entity& e = entities[i];
                e.dataIndex = i;
                e.bitset.reset();
                e.alive = false;
            }

            capacity = newCapacity;
        }

        void resizeIfNeeded()
        {
            if (capacity > newSize)
                return;

            resize(2 * capacity);
        }

        Entity& entity(EntityIndex index) noexcept
        {
            assert(newSize > index);

            return entities[index];
        }

        const Entity& entity(EntityIndex index) const noexcept
        {
            assert(newSize > index);

            return entities[index];
        }

    public:
        Manager(size_t initialCapacity = 100)
        {
            resize(initialCapacity);
        }

        bool isAlive(EntityIndex index) const noexcept
        {
            return entity(index).alive;
        }

        void kill(EntityIndex index) noexcept
        {
            entity(index).alive = false;
        }

        template <typename T>
        bool hasTag(EntityIndex index) const noexcept
        {
            static_assert(Settings::template isTag<T>(), "Type is not a tag");
            return entity(index).bitset[Settings::template tagBit<T>()];
        }

        template <typename T>
        void addTag(EntityIndex index) noexcept
        {
            static_assert(Settings::template isTag<T>(), "Type is not a tag");
            entity(index).bitset[Settings::template tagBit<T>()] = true;
        }

        template <typename T>
        void removeTag(EntityIndex index) noexcept
        {
            static_assert(Settings::template isTag<T>(), "Type is not a tag");
            entity(index).bitset[Settings::template tagBit<T>()] = false;
        }

        template <typename T>
        bool hasComponent(EntityIndex index) const noexcept
        {
            static_assert(Settings::template isComponent<T>(), "Type is not a component");
            return entity(index).bitset[Settings::template componentBit<T>()];
        }

        template <typename T, typename... TArgs>
        auto& addComponent(EntityIndex index, TArgs&&... args) noexcept
        {
            static_assert(Settings::template isComponent<T>(), "Type is not a component");

            Entity& e = entity(index);
            e.bitset[Settings::template componentBit<T>()] = true;

            auto& c(components.template component<T>(e.dataIndex));
            new (&c) T(SXI_MPL_FWD(args)...);
            return c;
        }

        template <typename T>
        void component(EntityIndex index) noexcept
        {
            static_assert(Settings::template isComponent<T>(), "Type is not a component");
            assert(hasComponent<T>(index));

            return components.template component<T>(entity(index).dataIndex);
        }

        template <typename T>
        void removeComponent(EntityIndex index) noexcept
        {
            static_assert(Settings::template isComponent<T>(), "Type is not a component");
            entity(index).bitset[Settings::template componentBit<T>()] = false;
        }

        EntityIndex createIndex()
        {
            resizeIfNeeded();

            EntityIndex freeIndex(newSize++);

            assert(!entities[freeIndex].alive);

            Entity& e = entities[freeIndex];
            e.alive = true;
            e.bitset.reset();

            return freeIndex;
        }

        void clear() noexcept
        {
            for (size_t i = 0; i < capacity; ++i)
            {
                Entity& e = entities[i];
                e.dataIndex = i;
                e.bitset.reset();
                e.alive = false;
            }

            size = newSize = 0;
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

        template <typename T>
        bool matchesSignature(EntityIndex index) const noexcept
        {
            static_assert(Settings::template isSignature<T>(), "Type is not a signature");

            const Bitset& entityBitset = entity(index).bitset;
            const Bitset& signatureBitset = signatureBitsets.template signatureBitset<T>();

            return (signatureBitset & entityBitset) == signatureBitset;
        }

        template <typename Func>
        void forEntities(Func&& func)
        {
            for (EntityIndex i = 0; i < size; ++i)
                func(i);
        }

        template <typename T, typename Func>
        void forEntitiesMatchingSignature(Func&& func)
        {
            static_assert(Settings::template isSignature<T>(), "Type is not a signature");

            forEntities([this, &func](EntityIndex i) {
                if (matchesSignature<T>(i))
                    expandSignatureCall<T>(i, func);
            });
        }

    private:
        template <typename... Ts>
        struct ExpandCallHelper;

        template <typename T, typename Func>
        void expandSignatureCall(EntityIndex index, Func&& func)
        {
            static_assert(Settings::template isSignature<T>(), "Type is not a signature");

            using RequiredComponents = typename Settings::SignatureBitsets::template SignatureComponents<T>;
            using Helper = mpl::Rename<ExpandCallHelper, RequiredComponents>;

            Helper::call(index, *this, func);
        }

        template <typename... Ts>
        struct ExpandCallHelper
        {
            template <typename Func>
            static void call(EntityIndex index, TManager& mgr, Func&& func)
            {
                DataIndex dataIndex = mgr.entity(index).dataIndex;
                func(index, mgr.components.template component<Ts>(di)...);
            }
        }

        size_t refreshImpl() noexcept
        {
            EntityIndex left{0}, right{newSize - 1};
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
                    --right;
                }

                assert(!entities[left].alive);
                assert(entities[right].alive);

                std::swap(entities[left++], entities[right--]);
            }
            return right;
        }
    
    public:
        size_t entityCount() const noexcept
        {
            return size;
        }

        size_t entityCapacity() const noexcept
        {
            return capacity;
        }

        auto print(std::ostream& os=std::cout) const 
        {
            os << "size: " << size
               << "\nnewSize: " << newSize
               << "\ncapacity: " << capacity << "\n";

            for (size_t i = 0; i < newSize; i++)
            {
                const Entity& e = entities[i];
                os << (e.alive ? "A" : "D");
            }

            os << "\n\n";
        }
    };
}