#pragma once

#include <stddef.h>

#include "TypeList.h"

namespace sxi::mpl
{
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
}