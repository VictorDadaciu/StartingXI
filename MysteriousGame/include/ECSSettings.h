#pragma once

#include "SXICore/ECS/Settings.h"
#include "SXICore/components/PositionComponent.h"
#include "SXICore/components/YRotationComponent.h"
#include "SXIRenderer/components/RenderComponent.h"
#include "SXIRenderer/tags/LightTag.h"

#include "SXICore/MPL/Filter.h"
#include "SXICore/MPL/Contains.h"

using Components = sxi::ecs::ComponentList<
    sxi::ecs::PositionComponent,
    sxi::ecs::YRotationComponent,
    sxi::ecs::RenderComponent>;

struct ObjectTag{};
using Tags = sxi::ecs::TagList<sxi::ecs::LightTag, ObjectTag>;

using Object = sxi::ecs::Archetype<
    sxi::ecs::PositionComponent,
    sxi::ecs::YRotationComponent,
    sxi::ecs::RenderComponent,
    ObjectTag>;
using Light = sxi::ecs::Archetype<
    sxi::ecs::PositionComponent,
    sxi::ecs::LightTag>;
using Archetypes = sxi::ecs::ArchetypeList<Object, Light>;

using RotateSignature = sxi::ecs::Signature<sxi::ecs::YRotationComponent, ObjectTag>;
using MoveSignature = sxi::ecs::Signature<sxi::ecs::PositionComponent, ObjectTag>;
using CalculateDescriptorsSignature = sxi::ecs::Signature<
    sxi::ecs::PositionComponent,
    sxi::ecs::YRotationComponent,
    sxi::ecs::RenderComponent>;
using LightSignature = sxi::ecs::Signature<
    sxi::ecs::PositionComponent,
    sxi::ecs::LightTag>;
using RenderSignature = sxi::ecs::Signature<sxi::ecs::RenderComponent>;
using Signatures = sxi::ecs::SignatureList<
    RotateSignature,
    MoveSignature,
    CalculateDescriptorsSignature,
    RenderSignature,
    LightSignature>;

using ECSSettings = sxi::ecs::Settings<Components, Tags, Archetypes, Signatures>;