#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <string>

#include <SXICore/Timing.h>
#include <SXICore/Types.h>
#include <SXICore/Exception.h>

#include "Texture.h"
#include "Model.h"
#include "Scene.h"
#include "detail/GraphicsPipeline.h"
#include "detail/RenderPass.h"
#include "detail/Utils.h"
#include "detail/Window.h"
#include "detail/Context.h"
#include "detail/Buffer.h"
#include "components/RenderComponent.h"

namespace sxi::renderer
{
    /**
     * @brief Initializes the renderer.
     * 
     * Must be destroyed before program end. Cannot be called more than once before
     * being destroyed.
     * 
     * @param uint32_t width: width of the window
     * @param uint32_t height: height of the window
     * @param bool releaseValidationLayers(=false): By default, validation layers are
     * 												disabled on release builds, set
     * 												this to true to force enable.
     */
    void init(u32, u32, bool=false);

    /**
     * @brief Pass in SPIRV shader code to construct graphics pipeline.
     * 
     * @param const std::vector<char>& vertCode: Bytes representing the contents of 
     *                                           the vertex shader of the desired
     *                                           pipeline.
     * @param const std::vector<char>& fragCode: Bytes representing the contents of 
     *                                           the fragment shader of the desired
     *                                           pipeline.
     */
    void addGraphicsPipeline(const std::vector<char>&, const std::vector<char>&);

    /**
     * @brief Pass in path to image to create a texture for models
     * 
     * @param const std::string& path: Path to an image file.
     */
    void addTexture(const std::string&);

    /**
     * @brief Pass in path to obj file to create a model
     * 
     * @param const std::string& path: Path to an obj file.
     */
    void addModel(const std::string&);

    namespace detail 
    {
        template <typename TSettings>
        void recordCommandBuffer(ecs::Manager<TSettings> mgr, VkCommandBuffer commandBuffer, u32 imageIndex, u32 currentFrame)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;
            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("Failed to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = detail::basicRenderPass->pass;
            renderPassInfo.framebuffer = detail::basicRenderPass->frameBuffers[imageIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = detail::window->swapchain->extent;
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = SXI_TO_U32(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, detail::basicLightingPipeline->pipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(detail::window->swapchain->extent.width);
            viewport.height = static_cast<float>(detail::window->swapchain->extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = detail::window->swapchain->extent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            const SceneData& sceneData = scene->currentSceneData();
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                detail::basicLightingPipeline->layout,
                detail::DescriptorSetType::PerFrame, 1,
                &sceneData.frameDescriptorSet, 0, nullptr);

            VkBuffer vertexBuffers[] = { detail::vertexBuffer->buffer };
            mgr.template forEntitiesMatching<ecs::Signature<sxi::ecs::RenderComponent>>([&](auto& entityIndex, auto& renderComponent){
                    Model* model = models[renderComponent.mdl];
                    VkDeviceSize offsets[] = { model->vertexBufferOffset };
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                    
                    vkCmdBindIndexBuffer(commandBuffer, detail::indexBuffer->buffer, model->indexBufferOffset, VK_INDEX_TYPE_UINT32);
                    vkCmdBindDescriptorSets(
                        commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        detail::basicLightingPipeline->layout,
                        detail::DescriptorSetType::PerModel, 1,
                        &textures[renderComponent.tex]->descriptorSet, 0, nullptr);

                    vkCmdBindDescriptorSets(
                        commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        detail::basicLightingPipeline->layout,
                        detail::DescriptorSetType::PerObject, 1,
                        &sceneData.objectDescriptorSets[entityIndex], 0, nullptr);
                        
                    vkCmdDrawIndexed(commandBuffer, SXI_TO_U32(model->indices.size()), 1, 0, 0, 0);	
                });

            vkCmdEndRenderPass(commandBuffer);

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
                throw InvalidArgumentException("Failed to record command buffer");
        }
    }

    /**
     * @brief Renders the frame to the screen.
     * 
     * Must be called only once per frame.
     */
    template <typename TSettings>
	void render(ecs::Manager<TSettings>& mgr, const Time& time)
	{
        scene->run(mgr, time);
            
		const detail::FrameContext* frameContext = detail::context->currentFrameContext();
		vkWaitForFences(detail::context->logicalDevice, 1, &frameContext->inFlightFence, VK_TRUE, UINT64_MAX);
		
		u32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(detail::context->logicalDevice, detail::window->swapchain->swapchain, UINT64_MAX, frameContext->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		
		vkResetFences(detail::context->logicalDevice, 1, &frameContext->inFlightFence);
			
		vkResetCommandBuffer(frameContext->commandBuffer, 0);
		detail::recordCommandBuffer(mgr, frameContext->commandBuffer, imageIndex, detail::context->currentFrame());

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { frameContext->imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frameContext->commandBuffer;

		VkSemaphore signalSemaphores[] = { detail::window->swapchain->renderFinishedSemaphores[imageIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(detail::context->graphicsQueue, 1, &submitInfo, frameContext->inFlightFence) != VK_SUCCESS)
			throw InvalidArgumentException("Failed to submit draw command buffer");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { detail::window->swapchain->swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(detail::context->presentQueue, &presentInfo);

		detail::context->advanceFrame();
	}

    /**
     * @brief Destroys the renderer and all associated data.
     */
    void destroy();
}