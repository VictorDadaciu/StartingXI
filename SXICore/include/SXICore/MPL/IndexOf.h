#pragma once

#include "TypeList.h"

#include <cstddef>
#include <stddef.h>
#include <type_traits>

namespace sxi::mpl
{
namespace detail
{
    template<typename T, class typelist>
    struct IndexOfHelper;

    template<typename T>
    struct IndexOfHelper<T, typelist<>>
    {
        static constexpr size_t value = -1;
    };

    template<typename T, typename... Tail>
    struct IndexOfHelper<T, typelist<T, Tail...>>
    {
        static constexpr size_t value = 0;
    };

    template<typename T, typename Head, typename... Tail>
    struct IndexOfHelper<T, typelist<Head, Tail...>>
    {
        static constexpr size_t value = 1 + IndexOfHelper<T, typelist<Tail...>>::value;
    };
} // namespace detail

template<typename T, class List>
using IndexOf = std::integral_constant<size_t, detail::IndexOfHelper<T, List>::value>;
} // namespace sxi::mpl