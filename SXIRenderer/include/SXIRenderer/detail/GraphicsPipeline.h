#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace sxi::renderer::detail
{
struct GraphicsPipeline
{
    VkPipelineLayout layout{};
    VkPipeline pipeline{};

    GraphicsPipeline(const std::vector<char>&, const std::vector<char>&);
    ~GraphicsPipeline();

private:
    void createLayout();
    void createPipeline(const std::vector<char>&, const std::vector<char>&);
};

extern GraphicsPipeline* basicLightingPipeline;
} // namespace sxi::renderer::detail