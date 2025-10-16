#include "Scene.h"

#include "detail/Window.h"
#include "detail/Buffer.h"

#include "SXICore/Exception.h"

namespace sxi::renderer
{
    Scene* scene{};

    SceneData::SceneData(u8 numObjects, u8 frame) : frame(frame)
    {
        objectDescriptorSets.resize(numObjects);
        createFrameDescriptorSet();
        createObjectDescriptorSets();
    }

    void Scene::moveLight(const Time& time)
    {
        static TimePoint start = time.time;
		float timePassed = -Time::elapsed(time.time, start);

        lightPos = glm::vec3(100.f * std::sinf(timePassed), 30, 100.f * std::cosf(timePassed));
    }

    void Scene::rotateObjects(const Time& time)
    {
        for (float& rotation : rotations)
            rotation += 0.1f * time.dt;
    }

    void Scene::finalize()
    {
        u8 currentFrame = detail::context->currentFrame();
        char* offset = (char*)detail::uniformBuffers[currentFrame].mapped;
        
        frameUBO.view = glm::lookAt(glm::vec3(0.f, 50.f, -75.f), glm::vec3(0.f, 20.f, 0.f), SXI_VEC3_UP);
        frameUBO.proj = glm::perspective(glm::radians(60.0f), detail::window->swapchain->extent.width / (float) detail::window->swapchain->extent.height, 0.1f, 10000.0f);
		frameUBO.proj[1][1] *= -1;
        memcpy(offset, &frameUBO, sizeof(FrameUBO));

        frameLight = FrameLight{ glm::vec4(lightPos.x, lightPos.y, lightPos.z, 1.f) };
        offset += sizeof(FrameUBO);
        memcpy(offset, &frameLight, sizeof(FrameLight));

        for (size_t i = 0; i < rotations.size(); ++i)
            objectUBOs[i].model = glm::rotate(glm::translate(glm::mat4(1.0f), translations[i]), -rotations[i], SXI_VEC3_UP);
        offset += sizeof(FrameLight);
        memcpy(offset, objectUBOs.data(), objectUBOs.size() * sizeof(ObjectUBO));
    }

    void SceneData::createFrameDescriptorSet()
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

    void SceneData::createObjectDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> pSetLayouts(objectDescriptorSets.size(),
            detail::context->descriptorSetLayouts[detail::DescriptorSetType::PerObject]);

        VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = detail::context->descriptorPool;
		allocInfo.descriptorSetCount = SXI_TO_U32(objectDescriptorSets.size());
		allocInfo.pSetLayouts = pSetLayouts.data();

        if (vkAllocateDescriptorSets(detail::context->logicalDevice, &allocInfo, objectDescriptorSets.data()) != VK_SUCCESS)
			throw MemoryAllocationException("Failed to allocate descriptor sets");
        
        for (size_t i = 0; i < objectDescriptorSets.size(); ++i)
        {
            VkDescriptorBufferInfo uboInfo;
            uboInfo.buffer = detail::uniformBuffers[frame].buffer;
            uboInfo.offset = detail::uniformBuffers[frame].offset;
            uboInfo.range = sizeof(ObjectUBO);
            detail::uniformBuffers[frame].offset += uboInfo.range;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = objectDescriptorSets[i];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pBufferInfo = &uboInfo;

            vkUpdateDescriptorSets(detail::context->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
        }

    }

    Scene::Scene(u8 numObjects)
    {
        lightPos = glm::vec4(0.f, 30.f, 100.f, 1.f);
        translations.resize(numObjects);
        for (size_t i = 0; i < numObjects; ++i)
            translations[i] = glm::vec3(-30.f + i * 60.f, 0.f, 0.f);
        rotations.resize(numObjects, 0.f);
        objectUBOs.resize(numObjects);
        for (size_t i = 0; i < sceneDatas.size(); ++i)
            sceneDatas[i] = SceneData(numObjects, SXI_TO_U8(i));
    }

    void Scene::run(const Time& time)
    {
        moveLight(time);
        rotateObjects(time);
        finalize();
    }
}