#pragma once

#include "stddef.h"
#include "SXICore/MPL/Macros.h"

namespace sxi::ecs
{
    SXI_MPL_STRONG_TYPEDEF(size_t, TextureIndex);
    SXI_MPL_STRONG_TYPEDEF(size_t, ModelIndex);

    struct RenderComponent final
    {   
        TextureIndex tex;
        ModelIndex mdl;
    };
}