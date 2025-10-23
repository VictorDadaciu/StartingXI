#include "Scene.h"

#include "detail/Window.h"
#include "detail/Buffer.h"

#include "SXICore/Exception.h"

namespace sxi::renderer
{
    Scene* scene{};

    SceneData::SceneData(u8 maxObjects, u8 frame) : frame(frame)
    {
        objectDescriptorSets.resize(maxObjects);
        createFrameDescriptorSet();
        createObjectDescriptorSets();
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

    Scene::Scene(u8 maxObjects)
    {
        objectUBOs.resize(maxObjects);
        for (size_t i = 0; i < sceneDatas.size(); ++i)
            sceneDatas[i] = SceneData(maxObjects, SXI_TO_U8(i));
    }
}