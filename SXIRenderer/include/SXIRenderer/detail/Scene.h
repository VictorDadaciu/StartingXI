#pragma once

#include "../ECSConfig.h"
#include "Buffer.h"
#include "Context.h"
#include "Window.h"

#include <SXICore/ECS/Manager.h>
#include <SXICore/ECS/Settings.h>
#include <SXICore/ECSConfig.h>
#include <SXICore/Timing.h>
#include <SXICore/Types.h>
#include <SXIMath/Mat.h>
#include <SXIMath/Vec.h>
#include <array>
#include <vector>

namespace sxi::renderer::detail
{
using namespace sxi::types;

struct alignas(64) FrameUBO
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct alignas(64) FrameLight
{
    alignas(16) glm::vec4 light;
};

struct alignas(64) ObjectUBO
{
    alignas(16) glm::mat4 model;
};

class Scene;

class SceneData
{
public:
    VkDescriptorSet frameDescriptorSet{};
    std::vector<VkDescriptorSet> objectDescriptorSets{};

    SceneData() = default;
    SceneData(size_t, u8);

private:
    void createFrameDescriptorSet();
    void createObjectDescriptorSets();

    u8 frame{};

    friend Scene;
};

class Scene
{
public:
    Scene(size_t);
    ~Scene() = default;

    template<typename TSettings>
    void run(ecs::Manager<TSettings>& mgr, const Time& time, size_t start, size_t end)
    {
        u8 currentFrame = context->currentFrame();
        mgr.template forEntitiesMatching<ecs::SXI_LightSignature>(
            [this, &time, &currentFrame](auto&, auto& posComponent)
            {
                static TimePoint start = time.time;
                float timePassed = -Time::elapsed(start, time.time);

                posComponent.pos = glm::vec3(100.f * std::sinf(timePassed), 30, 100.f * std::cosf(timePassed));

                char* offset = (char*)detail::uniformBuffers[currentFrame].mapped;

                this->frameUBO.view = glm::lookAt(glm::vec3(-4.5f, 3.5f, -4.5f), glm::vec3(0.f, 1.f, 0.f), SXI_VEC3_UP);
                this->frameUBO.proj = glm::perspective(glm::radians(60.0f),
                                                       detail::window->swapchain->extent.width /
                                                           (float)detail::window->swapchain->extent.height,
                                                       0.1f,
                                                       10000.0f);
                this->frameUBO.proj[1][1] *= -1;
                memcpy(offset, &this->frameUBO, sizeof(FrameUBO));

                this->frameLight =
                    FrameLight{glm::vec4(posComponent.pos.x, posComponent.pos.y, posComponent.pos.z, 1.f)};
                offset += sizeof(FrameUBO);
                memcpy(offset, &this->frameLight, sizeof(FrameLight));
            },
            start,
            end);

        char* offset = (char*)detail::uniformBuffers[currentFrame].mapped + sizeof(FrameUBO) + sizeof(FrameLight);
        mgr.template forEntitiesMatching<ecs::SXI_RenderObjectSignature>(
            [this](auto& entityIndex, auto& posComponent, auto& rotComponent, auto&)
            {
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), posComponent.pos);
                glm::mat4 rotation = glm::eulerAngleYXZ(rotComponent.rot.y, rotComponent.rot.x, rotComponent.rot.z);
                this->objectUBOs[entityIndex].model = translation * rotation;
            },
            start,
            end);
        memcpy(offset, objectUBOs.data(), objectUBOs.size() * sizeof(ObjectUBO));
    }

    inline const SceneData& currentSceneData() const { return sceneDatas[detail::context->currentFrame()]; }

private:
    FrameLight frameLight{};
    FrameUBO frameUBO{};
    std::vector<ObjectUBO> objectUBOs{};

    std::array<SceneData, detail::MAX_FRAMES_IN_FLIGHT> sceneDatas;
};

extern Scene* scene;
} // namespace sxi::renderer::detail