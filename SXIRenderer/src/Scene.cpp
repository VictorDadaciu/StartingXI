#include "Scene.h"

#include "detail/Window.h"
#include "detail/Buffer.h"

#include "SXICore/Exception.h"

namespace sxi::renderer
{
    Scene* scene{};

    SceneData::SceneData(u8 numObjects, u8 frame)
    {
        lightPos = glm::vec4(0.f, 20.f, 50.f, 1.f);
        createFrameDescriptorSet(frame);

        rotations.resize(numObjects, 0.f);
        objectUBOs.resize(numObjects);
        objectDescriptorSets.resize(numObjects);
        createObjectDescriptorSets(frame);

        FrameUBO frameUBO{};
        frameUBO.view = glm::lookAt(glm::vec3(50.f, 50.f, -50.f), glm::vec3(0.f, 20.f, 0.f), SXI_VEC3_UP);
        frameUBO.proj = glm::perspective(glm::radians(60.0f), detail::window->swapchain->extent.width / (float) detail::window->swapchain->extent.height, 0.1f, 10000.0f);
		frameUBO.proj[1][1] *= -1;
        memcpy(detail::uniformBuffers[frame].mapped, &frameUBO, sizeof(FrameUBO));
    }

    void SceneData::moveLight(const Time& time)
    {
        static TimePoint start = time.time;
		float timePassed = Time::elapsed(time.time, start);

        lightPos = glm::vec3(50.f * std::sinf(timePassed), 20, 50.f * std::cosf(timePassed));
    }

    void SceneData::rotateObjects(const Time& time)
    {
        for (float& rotation : rotations)
            rotation += 0.1f * time.dt;
    }

    void SceneData::begin(const SceneData* lastFrameSceneData)
    {
        lightPos = lastFrameSceneData->lightPos;
        rotations = lastFrameSceneData->rotations;
    }

    void SceneData::finalize()
    {
        u8 currentFrame = detail::context->currentFrame();

        frameLight = FrameLight{ glm::vec4(lightPos.x, lightPos.y, lightPos.z, 1.f) };
        memcpy((char*)detail::uniformBuffers[currentFrame].mapped + sizeof(FrameUBO), &frameLight, sizeof(FrameLight));

        static auto offset = sizeof(FrameUBO) + sizeof(FrameLight);
        for (size_t i = 0; i < rotations.size(); ++i)
            objectUBOs[i].model = glm::rotate(glm::mat4(1.0f), rotations[i] * glm::radians(-90.f), SXI_VEC3_UP);
        memcpy((char*)detail::uniformBuffers[currentFrame].mapped + offset, objectUBOs.data(), objectUBOs.size() * sizeof(ObjectUBO));
    }

    void SceneData::createFrameDescriptorSet(u8 frame)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = detail::context->descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &detail::context->descriptorSetLayouts[detail::DescriptorSetType::PerFrame];

        if (vkAllocateDescriptorSets(detail::context->logicalDevice, &allocInfo, &frameDescriptorSet) != VK_SUCCESS)
			throw MemoryAllocationException("Failed to allocate descriptor set");

        VkDescriptorBufferInfo uboInfo{};
        uboInfo.buffer = detail::uniformBuffers[frame].buffer;
        uboInfo.offset = 0;
        uboInfo.range = sizeof(FrameUBO);

        VkDescriptorBufferInfo lightInfo{};
        lightInfo.buffer = detail::uniformBuffers[frame].buffer;
        lightInfo.offset = sizeof(FrameUBO);
        lightInfo.range = sizeof(FrameLight);

        detail::uniformBuffers[frame].offset = sizeof(FrameUBO) + sizeof(FrameLight);

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = frameDescriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &uboInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = frameDescriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightInfo;

        vkUpdateDescriptorSets(detail::context->logicalDevice, SXI_TO_U32(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void SceneData::createObjectDescriptorSets(u8 frame)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = detail::context->descriptorPool;
		allocInfo.descriptorSetCount = SXI_TO_U32(objectDescriptorSets.size());
		allocInfo.pSetLayouts = &detail::context->descriptorSetLayouts[detail::DescriptorSetType::PerObject];

        if (vkAllocateDescriptorSets(detail::context->logicalDevice, &allocInfo, objectDescriptorSets.data()) != VK_SUCCESS)
			throw MemoryAllocationException("Failed to allocate descriptor sets");
        
        for (size_t i = 0; i < objectDescriptorSets.size(); ++i)
        {
            VkDescriptorBufferInfo uboInfo;
            uboInfo.buffer = detail::uniformBuffers[frame].buffer;
            uboInfo.offset = detail::uniformBuffers[frame].offset;
            uboInfo.range = sizeof(ObjectUBO);
            detail::uniformBuffers[frame].offset += sizeof(ObjectUBO);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = objectDescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &uboInfo;

            vkUpdateDescriptorSets(detail::context->logicalDevice, 1, &descriptorWrite, 0, nullptr);
        }
    }

    Scene::Scene(u8 numObjects)
    {
        for (size_t i = 0; i < sceneDatas.size(); ++i)
            sceneDatas[i] = new SceneData(numObjects, SXI_TO_U8(i));
    }

    Scene::~Scene()
    {
        for (SceneData* sceneData : sceneDatas)
            delete sceneData;
    }

    void Scene::run(const Time& time)
    {
        SceneData* currentSceneData = sceneDatas[detail::context->currentFrame()];
        currentSceneData->begin(sceneDatas[detail::context->lastFrame()]);
        currentSceneData->moveLight(time);
        currentSceneData->rotateObjects(time);
        currentSceneData->finalize();
    }
}