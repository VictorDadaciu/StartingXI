#pragma once

#include "PushFront.h"

#include <stddef.h>

namespace sxi::mpl
{
namespace detail
{
    template<size_t times, typename T>
    struct RepeatHelper;

    template<size_t times, typename T>
    struct RepeatHelper
    {
        using type = PushFront<T, typename RepeatHelper<times - 1, T>::type>::type;
    };

    template<typename T>
    struct RepeatHelper<1, T>
    {
        using type = typelist<T>;
    };

    template<typename T>
    struct RepeatHelper<0, T>
    {
        static_assert(false, "Cannot repeat type 0 times");
    };
} // namespace detail

template<size_t times, typename T>
using Repeat = typename detail::RepeatHelper<times, T>::type;
} // namespace sxi::mpl