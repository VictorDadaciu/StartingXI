#pragma once

#include <type_traits>

#include "../MPL/TypeList.h"
#include "../MPL/Contains.h"
#include "../MPL/Count.h"
#include "../MPL/IndexOf.h"
#include "../MPL/Filter.h"

namespace sxi::ecs
{
    template <typename... Ts> using Signature = sxi::mpl::typelist<Ts...>;
    template <typename... Ts> using SignatureList = sxi::mpl::typelist<Ts...>;

    template <typename... Ts> using ComponentList = sxi::mpl::typelist<Ts...>;
    template <typename... Ts> using TagList = sxi::mpl::typelist<Ts...>;

    template <typename... Ts> using Archetype = sxi::mpl::typelist<Ts...>;
    template <typename... Ts> using ArchetypeList = sxi::mpl::typelist<Ts...>;

    template <
        typename TComponentList,
        typename TTagList,
        typename TArchetypeList,
        typename TSignatureList
    >
    struct Settings
    {
        using ComponentList = TComponentList;
        using TagList = TTagList;
        using ArchetypeList = TArchetypeList;
        using SignatureList = TSignatureList;
        using TSettings = Settings<ComponentList, TagList, ArchetypeList, SignatureList>;

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
        static constexpr bool isArchetype() noexcept
        {
            return mpl::Contains<T, ArchetypeList>::value;
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

        static constexpr size_t archetypeCount() noexcept
        {
            return mpl::Count<ArchetypeList>::value;
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
        static constexpr size_t archetypeId() noexcept
        {
            return mpl::IndexOf<T, ArchetypeList>::value;
        }

        template <typename T>
        static constexpr size_t signatureId() noexcept
        {
            return mpl::IndexOf<T, SignatureList>::value;
        }

        template <typename TComponent>
        using IsComponentFilter = std::bool_constant<isComponent<TComponent>()>;

        template <typename TTag>
        using IsTagFilter = std::bool_constant<isTag<TTag>()>;

        template <typename TSignature>
        using SignatureComponents = mpl::Filter<IsComponentFilter, TSignature>;

        template <typename TSignature>
        using SignatureTags = mpl::Filter<IsTagFilter, TSignature>;
    };
}