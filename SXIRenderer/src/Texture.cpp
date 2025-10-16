#include "Texture.h"
#include "detail/Context.h"
#include "detail/Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cmath>
#include <iostream>

#include "SXICore/Types.h"
#include "SXICore/Exception.h"


namespace sxi::renderer
{
    std::vector<Texture*> textures{};

    using namespace detail;

    Texture::Texture(const std::string& path)
	{
        raw = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        size = SXI_TO_U64(width) * height * 4;
        mipLevels = SXI_TO_U32(std::floor(std::log2(std::max(width, height)))) + 1;
        if (!raw)
            throw InvalidArgumentException("Failed to load texture image");

        createVkResources();
	}

    Texture::~Texture()
    {
        vkDestroySampler(context->logicalDevice, sampler, nullptr);

		vkDestroyImageView(context->logicalDevice, imageView, nullptr);

		vkDestroyImage(context->logicalDevice, image, nullptr);
		vkFreeMemory(context->logicalDevice, memory, nullptr);

        stbi_image_free(raw);
    }

    void Texture::createVkResources()
    {
        VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMem;

		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMem);

		void* data;
		vkMapMemory(context->logicalDevice, stagingBufferMem, 0, size, 0, &data);
		memcpy(data, raw, SXI_TO_SIZE(size));
		vkUnmapMemory(context->logicalDevice, stagingBufferMem);

		createImage(
			width,
			height,
			mipLevels,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			image,
			memory);

		transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(stagingBuffer, image, SXI_TO_U32(width), SXI_TO_U32(height));
		generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, SXI_TO_I32(width), SXI_TO_I32(height), mipLevels);

		vkDestroyBuffer(context->logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(context->logicalDevice, stagingBufferMem, nullptr);

        imageView = createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = context->currentPhysicalDevice().properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f; // Optional
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.mipLodBias = 0.0f; // Optional

		if (vkCreateSampler(context->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create texture sampler");
	
        VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = detail::context->descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &detail::context->descriptorSetLayouts[detail::DescriptorSetType::PerModel];

        if (vkAllocateDescriptorSets(detail::context->logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
			throw MemoryAllocationException("Failed to allocate descriptor set");

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(detail::context->logicalDevice, 1, &descriptorWrite, 0, nullptr);
    }}