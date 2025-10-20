#pragma once

#include <tuple>

#include "Rename.h"

namespace sxi::mpl
{
    template <typename TypeList>
    using Tuple = Rename<std::tuple, TypeList>;
}