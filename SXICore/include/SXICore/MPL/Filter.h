#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
    namespace detail 
    {
        template <template <typename> class Pred, typename... List> struct Filter;
        
        template <template <typename> class Pred, typename Head, typename... Tail>
        struct Filter<Pred, typelist<Head, Tail...>>
        {
            using type = std::conditional_t<Pred<Head>::value,
                                            typename PushFront<Head, typename Filter<Pred, typelist<Tail...>>::type>::type,
                                            typename Filter<Pred, typelist<Tail...>>::type>;
        };
        
        template <template <typename> class Pred, typename Head>
        struct Filter<Pred, typelist<Head>>
        {
            using type = std::conditional_t<Pred<Head>::value,
                                            typelist<Head>,
                                            typelist<>>;
        };
        
        template <template <typename> class Pred>
        struct Filter<Pred, typelist<>>
        {
            using type = typelist<>;
        };
    }

    template <template <typename> class Pred, typename TypeList>
    using Filter = typename detail::Filter<Pred, TypeList>::type;
}