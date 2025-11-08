#pragma once

#include "SXICore/ECS/Settings.h"
#include "SXICore/ECSConfig.h"
#include "SXICore/MPL/Macros.h"
#include "stddef.h"

namespace sxi::ecs
{
// Tags
struct LightTag final
{
};

using SXI_RendererTags = TagList<LightTag>;

// Components
SXI_MPL_STRONG_TYPEDEF(size_t, TextureIndex);
SXI_MPL_STRONG_TYPEDEF(size_t, ModelIndex);

struct RenderComponent final
{
    TextureIndex tex;
    ModelIndex mdl;
};

using SXI_RendererComponents = ComponentList<RenderComponent>;

// Signatures
using SXI_LightSignature = Signature<PositionComponent, LightTag>;
using SXI_RenderObjectSignature = Signature<PositionComponent, RotationComponent, RenderComponent>;

using SXI_RendererSignatures = SignatureList<SXI_RenderObjectSignature, SXI_LightSignature>;

// Archetypes
using Light = Archetype<PositionComponent, LightTag>;

using SXI_RendererArchetypes = ArchetypeList<Light>;
} // namespace sxi::ecs