#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "SXICore/Types.h"

namespace sxi::renderer::detail
{
    u32 findMemoryType(u32, VkMemoryPropertyFlags);

    VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

    void createImage(u32, u32, u32, VkSampleCountFlagBits, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);

    VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags, u32);

    void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout, u32);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(const VkCommandBuffer&);
}