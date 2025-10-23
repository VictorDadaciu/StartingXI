#pragma once

#include "detail/Context.h"

#include <array>
#include <vector>

#include <SXIMath/Mat.h>
#include <SXIMath/Vec.h>
#include <SXICore/Timing.h>
#include <SXICore/Types.h>
#include <SXICore/ECS/Manager.h>

#include <SXICore/components/PositionComponent.h>
#include <SXICore/components/YRotationComponent.h>
#include "components/RenderComponent.h"
#include "tags/LightTag.h"

#include "detail/Window.h"
#include "detail/Buffer.h"

#include "SXICore/Exception.h"

namespace sxi::renderer
{
    struct alignas(64) FrameUBO {
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

    struct alignas(64) FrameLight {
		alignas(16) glm::vec4 light;
    };

    struct alignas(64) ObjectUBO {
        alignas(16) glm::mat4 model;
    };

    class Scene;
    class SceneData
    {
    public:
        VkDescriptorSet frameDescriptorSet{};
        std::vector<VkDescriptorSet> objectDescriptorSets{};

        SceneData() = default;
        SceneData(u8, u8);

    private:
        void createFrameDescriptorSet();
        void createObjectDescriptorSets();
    
        u8 frame{};
    
        friend Scene;
};

class Scene
{
    public:
        Scene(u8);
        ~Scene() = default;
    
        template <typename TSettings>
        void run(ecs::Manager<TSettings>& mgr, const Time& time)
        {
            u8 currentFrame = detail::context->currentFrame();
            mgr.template forEntitiesMatching<ecs::Signature<ecs::PositionComponent, ecs::LightTag>>([this, &time, &currentFrame](auto&, auto& posComponent){
                static TimePoint start = time.time;
		        float timePassed = -Time::elapsed(time.time, start);

                posComponent.pos = glm::vec3(100.f * std::sinf(timePassed), 30, 100.f * std::cosf(timePassed));

                char* offset = (char*)detail::uniformBuffers[currentFrame].mapped;
        
                this->frameUBO.view = glm::lookAt(glm::vec3(0.f, 50.f, -75.f), glm::vec3(0.f, 20.f, 0.f), SXI_VEC3_UP);
                this->frameUBO.proj = glm::perspective(glm::radians(60.0f), detail::window->swapchain->extent.width / (float) detail::window->swapchain->extent.height, 0.1f, 10000.0f);
                this->frameUBO.proj[1][1] *= -1;
                memcpy(offset, &this->frameUBO, sizeof(FrameUBO));

                this->frameLight = FrameLight{ glm::vec4(posComponent.pos.x, posComponent.pos.y, posComponent.pos.z, 1.f) };
                offset += sizeof(FrameUBO);
                memcpy(offset, &this->frameLight, sizeof(FrameLight));
            });

            char* offset = (char*)detail::uniformBuffers[currentFrame].mapped + sizeof(FrameUBO) + sizeof(FrameLight);
            mgr.template forEntitiesMatching<ecs::Signature<
                ecs::PositionComponent,
                ecs::YRotationComponent,
                ecs::RenderComponent>>([this](auto& entityIndex, auto& posComponent, auto& rotComponent, auto&){
                    this->objectUBOs[entityIndex].model = glm::rotate(glm::translate(glm::mat4(1.0f), posComponent.pos), rotComponent.rot, SXI_VEC3_UP);
                });
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
}