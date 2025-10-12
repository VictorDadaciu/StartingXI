#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace sxi::renderer::detail
{
    struct RenderPass
    {
        VkRenderPass pass{};
		VkImage colorImage{};
		VkDeviceMemory colorImageMem{};
		VkImageView colorImageView{};
		VkImage depthImage{};
		VkDeviceMemory depthImageMem{};
		VkImageView depthImageView{};
        std::vector<VkFramebuffer> frameBuffers{};

        RenderPass();
        ~RenderPass();

    private:
        void createRenderPass();
        void createResources();
        void createFrameBuffers();
    };
    extern RenderPass* basicRenderPass;
}