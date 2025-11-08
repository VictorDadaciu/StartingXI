#pragma once

#include "SXICore/ECS/Settings.h"
#include "SXICore/ECSConfig.h"
#include "SXIRenderer/ECSConfig.h"

using Components = sxi::ecs::FinalizeList<sxi::ecs::SXI_CoreComponents, sxi::ecs::SXI_RendererComponents>;

struct ObjectTag
{
};

using Tags = sxi::ecs::FinalizeList<sxi::ecs::SXI_RendererTags, sxi::ecs::TagList<ObjectTag>>;

using Object =
    sxi::ecs::Archetype<sxi::ecs::PositionComponent, sxi::ecs::RotationComponent, sxi::ecs::RenderComponent, ObjectTag>;
using Archetypes = sxi::ecs::FinalizeList<sxi::ecs::SXI_RendererArchetypes, sxi::ecs::ArchetypeList<Object>>;

using RotateSignature = sxi::ecs::Signature<sxi::ecs::RotationComponent, ObjectTag>;
using MoveSignature = sxi::ecs::Signature<sxi::ecs::PositionComponent, ObjectTag>;

using Signatures =
    sxi::ecs::FinalizeList<sxi::ecs::SXI_RendererSignatures, sxi::ecs::SignatureList<RotateSignature, MoveSignature>>;

using ECSSettings = sxi::ecs::Settings<Components, Tags, Archetypes, Signatures>;