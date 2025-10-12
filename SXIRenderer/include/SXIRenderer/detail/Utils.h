#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "SXICore/Types.h"

namespace sxi::renderer::detail
{
    u32 findMemoryType(u32, VkMemoryPropertyFlags);

    VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

    void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);

    void createImage(u32, u32, u32, VkSampleCountFlagBits, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);

    VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags, u32);

    void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout, u32);

    void copyBufferToImage(VkBuffer, VkImage, u32, u32);

    void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize, VkDeviceSize);

    void generateMipmaps(VkImage, VkFormat, i32, i32, u32);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(const VkCommandBuffer&);

    VkShaderModule createShaderModule(const std::vector<char>&);
}