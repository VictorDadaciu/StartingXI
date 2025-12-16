#pragma once

#include "IsSame.h"
#include "TypeList.h"

#include <type_traits>

namespace sxi::mpl
{
namespace detail
{
    template<typename T, class typelist>
    struct ContainsHelper;

    template<typename T>
    struct ContainsHelper<T, typelist<>>
    {
        static constexpr bool value = false;
    };

    template<typename T, typename... Tail>
    struct ContainsHelper<T, typelist<T, Tail...>>
    {
        static constexpr bool value = true;
    };

    template<typename T, typename Head, typename... Tail>
    struct ContainsHelper<T, typelist<Head, Tail...>>
    {
        static constexpr bool value = IsSame<T, Head>::value || ContainsHelper<T, typelist<Tail...>>::value;
    };
} // namespace detail

template<typename T, class List>
using Contains = std::bool_constant<detail::ContainsHelper<T, List>::value>;

} // namespace sxi::mpl