#pragma once

#include "ECS/Settings.h"
#include "SXIMath/Vec.h" // IWYU pragma: keep

namespace sxi::ecs
{
struct PositionComponent final
{
    glm::vec3 pos;
};

struct RotationComponent final
{
    glm::vec3 rot;
};

using SXI_CoreComponents = ComponentList<PositionComponent, RotationComponent>;

} // namespace sxi::ecs