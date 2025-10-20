#pragma once

#include <stddef.h>

#include "TypeList.h"

namespace sxi::mpl
{
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