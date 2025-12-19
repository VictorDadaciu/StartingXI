#include "detail/Buffer.h"

#include "detail/Context.h"
#include "detail/Model.h"
#include "detail/Scene.h"
#include "detail/Utils.h"

namespace sxi::renderer::detail
{
VertexBuffer* vertexBuffer{};
IndexBuffer* indexBuffer{};
UniformBuffer* uniformBuffers{};

constexpr VkDeviceSize MAX_VERTICES = 1000000;
constexpr VkDeviceSize MAX_INDICES = 1000000;
constexpr VkDeviceSize MAX_OBJECTS = 5000;

VertexBuffer::VertexBuffer() : size(sizeof(Vertex) * MAX_VERTICES), offset(0)
{
    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 buffer,
                 memory);
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(context->logicalDevice, buffer, nullptr);
    vkFreeMemory(context->logicalDevice, memory, nullptr);
}

IndexBuffer::IndexBuffer() : size(sizeof(u32) * MAX_INDICES), offset(0)
{
    createBuffer(size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 buffer,
                 memory);
}

IndexBuffer::~IndexBuffer()
{
    vkDestroyBuffer(context->logicalDevice, buffer, nullptr);
    vkFreeMemory(context->logicalDevice, memory, nullptr);
}

UniformBuffer::UniformBuffer() :
    size(sizeof(detail::FrameUBO) + sizeof(detail::FrameLight) + sizeof(detail::ObjectUBO) * MAX_OBJECTS), offset(0)
{
    createBuffer(size,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 buffer,
                 memory);
    vkMapMemory(context->logicalDevice, memory, 0, size, 0, &mapped);
}

UniformBuffer::~UniformBuffer()
{
    vkDestroyBuffer(context->logicalDevice, buffer, nullptr);
    vkFreeMemory(context->logicalDevice, memory, nullptr);
}
} // namespace sxi::renderer::detail