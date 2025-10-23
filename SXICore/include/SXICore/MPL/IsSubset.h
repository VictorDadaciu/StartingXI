#pragma once

#include "Contains.h"

namespace sxi::mpl
{
    template <typename List, typename Head, typename... Tail>
    struct IsSubset;
    
    template <typename List, typename Head, typename... Tail>
    struct IsSubset<List, typelist<Head, Tail...>>
    {
        static constexpr bool value = Contains<Head, List>::value && IsSubset<List, typelist<Tail...>>::value;
    };
    
    template <typename List, typename Head>
    struct IsSubset<List, typelist<Head>>
    {
        static constexpr bool value = Contains<Head, List>::value;
    };
    
    template <typename List>
    struct IsSubset<List, typelist<>>
    {
        static constexpr bool value = true;
    };

    using Set = typelist<int, short, char, long>;

    using WillPass = typelist<char, short>;
    using WillFail = typelist<char, float>;

    static_assert(IsSubset<Set, WillPass>::value);
    static_assert(!IsSubset<Set, WillFail>::value);
}