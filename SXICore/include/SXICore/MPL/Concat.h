#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
namespace detail
{
    template <typename... Ts>
    struct Concat;

    template <typename... First, typename... Second, typename... List>
    struct Concat<typelist<First...>, typelist<Second...>, List...>
    {
        using type = typename Concat<typelist<First..., Second...>, List...>::type;
    };

    template <typename... First, typename... Second>
    struct Concat<typelist<First...>, typelist<Second...>>
    {
        using type = typelist<First..., Second...>;
    };

    template <typename... Ts>
    struct Concat<typelist<Ts...>, typelist<>>
    {
        using type = typelist<Ts...>;
    };
} // namespace detail

template <typename... Ts>
using Concat = typename detail::Concat<Ts...>::type;
} // namespace sxi::mpl