#pragma once

#include "PushFront.h"

#include <type_traits>

namespace sxi::mpl
{
namespace detail
{
    template<template<typename> class Pred, typename... List>
    struct FilterHelper;

    template<template<typename> class Pred, typename Head, typename... Tail>
    struct FilterHelper<Pred, typelist<Head, Tail...>>
    {
        using type = std::conditional_t<Pred<Head>::value,
                                        PushFront<Head, typename FilterHelper<Pred, typelist<Tail...>>::type>,
                                        typename FilterHelper<Pred, typelist<Tail...>>::type>;
    };

    template<template<typename> class Pred, typename Head>
    struct FilterHelper<Pred, typelist<Head>>
    {
        using type = std::conditional_t<Pred<Head>::value, typelist<Head>, typelist<>>;
    };

    template<template<typename> class Pred>
    struct FilterHelper<Pred, typelist<>>
    {
        using type = typelist<>;
    };
} // namespace detail

template<template<typename> class Pred, typename TypeList>
using Filter = typename detail::FilterHelper<Pred, TypeList>::type;
} // namespace sxi::mpl