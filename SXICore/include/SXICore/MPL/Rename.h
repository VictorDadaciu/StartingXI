#pragma once

#include "TypeList.h"

namespace sxi::mpl
{
namespace detail
{
    template<template<typename...> class NewName, typename... List>
    struct RenameHelper;

    template<template<typename...> class NewName, typename... List>
    struct RenameHelper<NewName, typelist<List...>>
    {
        using type = NewName<List...>;
    };
} // namespace detail

template<template<typename...> class NewName, typename TypeList>
using Rename = typename detail::RenameHelper<NewName, TypeList>::type;
} // namespace sxi::mpl