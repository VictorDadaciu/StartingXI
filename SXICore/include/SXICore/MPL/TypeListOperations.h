#pragma once

#include <stddef.h>
#include <tuple>

#include "Type.h"
#include "Map.h"
#include "Macros.h"

namespace sxi::mpl 
{  
    namespace detail
    {
        template <typename Func, typename TTuple, std::size_t... Indices>
        constexpr decltype(auto) tupleApplyImpl(Func&& func, TTuple&& tuple, std::index_sequence<Indices...>)
        {
            return SXI_MPL_FWD(func)(std::get<Indices>(SXI_MPL_FWD(tuple))...);
        }

        template <typename Func, typename TTuple>
        constexpr decltype(auto) tupleApply(Func&& func, TTuple&& tuple)
        {
            using Indices = std::make_index_sequence<std::tuple_size<std::decay_t<TTuple>>::value>;
            return tupleApplyImpl(SXI_MPL_FWD(func), SXI_MPL_FWD(tuple), Indices{});
        }

        template <typename Func, typename... Ts>
        constexpr decltype(auto) forArgs(Func&& func, Ts&&... args)
        {
            return (void)std::initializer_list<int>{(func(SXI_MPL_FWD(args)), 0)...};
        }

        template <typename Func, typename TTuple>
        constexpr decltype(auto) forTuple(Func&& func, TTuple&& tuple)
        {
            return tupleApply(
                [&func](auto&&... xs)
                {
                    forArgs(func, SXI_MPL_FWD(xs)...);
                },
                SXI_MPL_FWD(tuple));
        }
    }

    template <typename Func, typename TTuple>
    constexpr decltype(auto) forTuple(
        Func&& func, TTuple&& tuple)
    {
        return detail::forTuple(func, SXI_MPL_FWD(tuple));  
    }

    template <typename List, typename Func>
    constexpr void forTypes(Func&& func) noexcept
    {
        forTuple(func, Tuple<Map<Type, List>>{});
    }
}