#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
    namespace detail
    {
        template <typename T, typename... List> struct PushFront;
        
        template <typename T, typename... List>
        struct PushFront<T, typelist<List...>>
        {
            using type = typelist<T, List...>;
        };
        
        template <typename T>
        struct PushFront<T, typelist<>>
        {
            using type = typelist<T>;
        };
    }

    template <typename T, typename TypeList>
    using PushFront = typename detail::PushFront<T, TypeList>::type;
}