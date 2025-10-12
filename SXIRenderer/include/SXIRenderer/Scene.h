#pragma once

#include "detail/Context.h"

#include <array>
#include <vector>

#include <SXIMath/Mat.h>
#include <SXIMath/Vec.h>
#include <SXICore/Timing.h>
#include <SXICore/Types.h>

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

        SceneData(u8, u8);

        void moveLight(const Time&);
        void rotateObjects(const Time&);

    private:
        void begin(const SceneData*);
        void finalize();

        void createFrameDescriptorSet(u8);
        void createObjectDescriptorSets(u8);

        std::vector<float> rotations{};
        glm::vec3 lightPos{};

        FrameLight frameLight{};
        FrameUBO frameUBO{};
        std::vector<ObjectUBO> objectUBOs{};

        friend Scene;
    };

    class Scene
    {
    public:
        Scene(u8);
        ~Scene();

        void run(const Time&);

        inline const SceneData* currentSceneData() const { return sceneDatas[detail::context->currentFrame()]; }
    private:
        std::array<SceneData*, detail::MAX_FRAMES_IN_FLIGHT> sceneDatas{};
    };

    extern Scene* scene;
}