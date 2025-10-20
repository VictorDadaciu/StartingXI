#pragma once

#include "TypeList.h"
#include "IsSame.h"

namespace sxi::mpl
{
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
}