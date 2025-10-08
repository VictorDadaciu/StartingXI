#pragma once

#include <vulkan/vulkan.h>

#include "SXICore/Types.h"

namespace sxi::renderer::detail
{
    VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags, u32);
}