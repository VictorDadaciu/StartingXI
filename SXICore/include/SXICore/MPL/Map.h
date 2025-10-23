#pragma once

#include "PushFront.h"

namespace sxi::mpl
{
    namespace detail
    {
        template <template <typename> typename Func, typename List> struct Map;

        template <template <typename> typename Func, typename Head, typename... Tail>
        struct Map<Func, typelist<Head, Tail...>>
        {
            using type = PushFront<Func<Head>, typename Map<Func, typelist<Tail...>>::type>::type;
        };

        template <template <typename> typename Func, typename Head>
        struct Map<Func, typelist<Head>>
        {
            using type = typelist<Func<Head>>;
        };

        template <template <typename> typename Func>
        struct Map<Func, typelist<>>
        {
            using type = typelist<>;
        };
    }

    template <template <typename> class Func, typename TypeList>
    using Map = typename detail::Map<Func, TypeList>::type;
}