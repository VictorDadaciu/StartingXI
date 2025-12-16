#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
namespace detail
{
    template<typename T, typename... List>
    struct PushFrontHelper;

    template<typename T, typename... List>
    struct PushFrontHelper<T, typelist<List...>>
    {
        using type = typelist<T, List...>;
    };

    template<typename T>
    struct PushFrontHelper<T, typelist<>>
    {
        using type = typelist<T>;
    };
} // namespace detail

template<typename T, typename TypeList>
using PushFront = typename detail::PushFrontHelper<T, TypeList>::type;
} // namespace sxi::mpl