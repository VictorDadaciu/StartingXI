#pragma once

#include <stddef.h>

namespace sxi::mpl 
{
    struct null_t{};
    
    template <typename... Ts>
    struct typelist{};

    template <typename T, typename U>
    struct IsSame
    {
        static constexpr bool value = false;
    };

    template <typename T>
    struct IsSame<T, T>
    {
        static constexpr bool value = true;
    };

    template <typename T, class typelist> struct Contains;

    template <typename T>
    struct Contains<T, typelist<>>
    {
        static constexpr bool value = false;
    };

    template <typename T, typename... Tail>
    struct Contains<T, typelist<T, Tail...>>
    {
        static constexpr bool value = true;
    };

    template <typename T, typename Head, typename... Tail>
    struct Contains<T, typelist<Head, Tail...>>
    {
        static constexpr bool value = IsSame<T, Head>::value || Contains<T, typelist<Tail...>>::value;
    };

    template <class typelist> struct Count;

    template <>
    struct Count<typelist<>>
    {
        static constexpr size_t value = 0;
    };

    template <typename Head, typename... Tail>
    struct Count<typelist<Head, Tail...>>
    {
        static constexpr size_t value = 1 + Count<typelist<Tail...>>::value;
    };

    template <typename T, class typelist> struct IndexOf;

    template <typename T>
    struct IndexOf<T, typelist<>>
    {
        static constexpr size_t value = -1;
    };

    template <typename T, typename... Tail>
    struct IndexOf<T, typelist<T, Tail...>>
    {
        static constexpr size_t value = 0;
    };

    template <typename T, typename Head, typename... Tail>
    struct IndexOf<T, typelist<Head, Tail...>>
    {
        static constexpr size_t value = 1 + IndexOf<T, typelist<Tail...>>::value;
    };
}