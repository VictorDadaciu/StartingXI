#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
    namespace detail
    {
        template <template <typename...> class NewName, typename... List> struct Rename;
        
        template <template <typename...> class NewName, typename... List>
        struct Rename<NewName, typelist<List...>>
        {
            using type = NewName<List...>;
        };
    }

    template <template <typename...> class NewName, typename TypeList>
    using Rename = typename detail::Rename<NewName, TypeList>::type;
}