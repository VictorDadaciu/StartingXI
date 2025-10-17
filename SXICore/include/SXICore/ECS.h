#pragma once

#include <iostream>
#include <type_traits>
#include <bitset>

#include "MPL.h"

namespace sxi::ecs
{   
    template <
        typename TComponentList,
        typename TTagList,
        typename TSignatureList
    >
    struct ECSSettings;
    
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
    struct ECSSettings
    {
        using ComponentList = typename TComponentList::typelist;
        using TagList = typename TTagList::typelist;
        using SignatureList = typename TSignatureList::typelist;
        using Settings_t = ECSSettings<ComponentList, TagList, SignatureList>;

        using SignatureBitSets = typename detail::SignatureBitsets<Settings_t>;
        using SignatureBitSetsStorage = typename detail::SignatureBitsetsStorage<Settings_t>;

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
}