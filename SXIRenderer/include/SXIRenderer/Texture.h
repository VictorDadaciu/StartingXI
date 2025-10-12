#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "SXICore/Types.h"

namespace sxi::renderer
{
    struct Texture
	{
		unsigned char* raw{};
		u64 size;
		int width{};
		int height{};
		int channels{};
		u32 mipLevels{};
		VkDescriptorSet descriptorSet{};

        VkImage image{};
        VkImageView imageView{};
        VkDeviceMemory memory{};
        VkSampler sampler{};

		Texture(const std::string&);
		Texture(Texture&) = delete;
		Texture(Texture&&) = default;

		Texture& operator=(Texture&) = delete;
		Texture& operator=(Texture&&) = default;

		~Texture();

	private:
        void createVkResources();
	};

    extern Texture* tex;
}