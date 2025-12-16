#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
namespace detail
{
    template<typename... Ts>
    struct ConcatHelper;

    template<typename... First, typename... Second, typename... List>
    struct ConcatHelper<typelist<First...>, typelist<Second...>, List...>
    {
        using type = typename ConcatHelper<typelist<First..., Second...>, List...>::type;
    };

    template<typename... First, typename... Second>
    struct ConcatHelper<typelist<First...>, typelist<Second...>>
    {
        using type = typelist<First..., Second...>;
    };

    template<typename... Ts>
    struct ConcatHelper<typelist<Ts...>, typelist<>>
    {
        using type = typelist<Ts...>;
    };
} // namespace detail

template<typename... Ts>
using Concat = typename detail::ConcatHelper<Ts...>::type;
} // namespace sxi::mpl