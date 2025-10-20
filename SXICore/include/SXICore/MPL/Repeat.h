#pragma once

#include <stddef.h>

#include "PushFront.h"

namespace sxi::mpl
{
    namespace detail
    {

        template <size_t times, typename T> struct Repeat;
        
        template <size_t times, typename T>
        struct Repeat
        {
            using type = PushFront<T, typename Repeat<times - 1, T>::type>::type;
        };
        
        template <typename T>
        struct Repeat<1, T>
        {
            using type = typelist<T>;
        };
        
        template <typename T>
        struct Repeat<0, T>
        {
            static_assert(false, "Cannot repeat type 0 times");
        };
    }

    template <size_t times, typename T>
    using Repeat = typename detail::Repeat<times, T>::type;
}