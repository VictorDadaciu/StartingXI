#pragma once

#include "Contains.h"
#include "IsSubset.h"
#include "PushFront.h"
#include "TypeList.h"

#include <type_traits>

namespace sxi::mpl
{
namespace detail
{
    template<typename List, typename Head, typename... Tail>
    struct SupersetsHelper;

    template<typename List, typename Head, typename... Tail>
    struct SupersetsHelper<List, typelist<Head, Tail...>>
    {
        using type = std::conditional_t<IsSubset<Head, List>::value,
                                        PushFront<Head, typename SupersetsHelper<List, typelist<Tail...>>::type>,
                                        typename SupersetsHelper<List, typelist<Tail...>>::type>;
    };

    template<typename List, typename Head>
    struct SupersetsHelper<List, typelist<Head>>
    {
        using type = std::conditional_t<IsSubset<Head, List>::value, typelist<Head>, typelist<>>;
    };

    template<typename List>
    struct SupersetsHelper<List, typelist<>>
    {
        using type = typelist<>;
    };
} // namespace detail

template<typename Subset, typename PossibleSupersets>
using Supersets = typename detail::SupersetsHelper<Subset, PossibleSupersets>::type;

namespace test
{
    using Lock1 = typelist<int, char>;
    using Lock2 = typelist<long, bool>;
    using Lock3 = typelist<int, double>;
    using Locks = typelist<Lock1, Lock2, Lock3>;

    using Key1 = typelist<int>;
    using Key2 = typelist<long, bool>;

    using SupersetsKey1 = Supersets<Key1, Locks>;
    using SupersetsKey2 = Supersets<Key2, Locks>;

    static_assert(Contains<Lock1, SupersetsKey1>::value);
    static_assert(!Contains<Lock2, SupersetsKey1>::value);
    static_assert(Contains<Lock3, SupersetsKey1>::value);
    static_assert(!Contains<Lock1, SupersetsKey2>::value);
    static_assert(Contains<Lock2, SupersetsKey2>::value);
    static_assert(!Contains<Lock1, SupersetsKey2>::value);
} // namespace test
} // namespace sxi::mpl