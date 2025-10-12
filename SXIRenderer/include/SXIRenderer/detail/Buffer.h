#pragma once

#include <vulkan/vulkan.h>

namespace sxi::renderer::detail
{
    struct VertexBuffer
    {
        VkBuffer buffer{};
        VkDeviceMemory memory{};
        VkDeviceSize size{};
        VkDeviceSize offset{};

        VertexBuffer();
        ~VertexBuffer();
    };

    struct IndexBuffer
    {
        VkBuffer buffer{};
        VkDeviceMemory memory{};
        VkDeviceSize size{};
        VkDeviceSize offset{};

        IndexBuffer();
        ~IndexBuffer();
    };

    struct UniformBuffer
    {
        VkBuffer buffer{};
        VkDeviceMemory memory{};
        VkDeviceSize size{};
        VkDeviceSize offset{};
        void* mapped{};

        UniformBuffer();
        ~UniformBuffer();
    };

    extern VertexBuffer* vertexBuffer;
    extern IndexBuffer* indexBuffer;
    extern UniformBuffer* uniformBuffers;
}