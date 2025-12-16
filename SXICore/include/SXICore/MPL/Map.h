#pragma once

#include "PushFront.h"

namespace sxi::mpl
{
namespace detail
{
    template<template<typename> typename Func, typename List>
    struct MapHelper;

    template<template<typename> typename Func, typename Head, typename... Tail>
    struct MapHelper<Func, typelist<Head, Tail...>>
    {
        using type = PushFront<Func<Head>, typename MapHelper<Func, typelist<Tail...>>::type>::type;
    };

    template<template<typename> typename Func, typename Head>
    struct MapHelper<Func, typelist<Head>>
    {
        using type = typelist<Func<Head>>;
    };

    template<template<typename> typename Func>
    struct MapHelper<Func, typelist<>>
    {
        using type = typelist<>;
    };
} // namespace detail

template<template<typename> class Func, typename TypeList>
using Map = typename detail::MapHelper<Func, TypeList>::type;
} // namespace sxi::mpl