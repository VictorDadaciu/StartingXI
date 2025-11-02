#pragma once

#include "Rename.h"

#include <tuple>

namespace sxi::mpl
{
template <typename TypeList>
using Tuple = Rename<std::tuple, TypeList>;
}