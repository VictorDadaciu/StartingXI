#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Scene.h"
#include "detail/Context.h"
#include "detail/Window.h"
#include "detail/RenderPass.h"
#include "detail/GraphicsPipeline.h"
#include "detail/Buffer.h"
#include "detail/Utils.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>

#include "SXICore/Exception.h"

namespace sxi::renderer
{
	const static uint8_t MAX_FRAMES_IN_FLIGHT = 2;
#ifdef NDEBUG
	static bool enableValidationLayers = false;
#else
	static bool enableValidationLayers = true;
#endif

	static VkDebugUtilsMessengerEXT debugMessenger{};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
		{
			std::cout << "Validation layer: " << pCallbackData->pMessage << std::endl;
		}

		return VK_FALSE;
	}

	static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		} 
		else 
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) 
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) 
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	static VkInstance createInstance(const std::vector<const char*>& validationLayers)
	{
		VkInstance instance;

		u32 extensionsCount{};
		const char* const * SDL_extensions = SDL_Vulkan_GetInstanceExtensions(&extensionsCount);
		std::vector<const char*> extensions(SDL_extensions, SDL_extensions + extensionsCount);
		if (enableValidationLayers)
		{
			if (!checkValidationLayerSupport(validationLayers))
				throw InitializationException("Vulkan validation layers not supported by installed version.");

			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Mysterious Game";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Starting XI";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = SXI_TO_U32(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = SXI_TO_U32(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			throw InitializationException("Failed to create Vulkan instance");

		return instance;
	}
	
	static void setupDebugCallback(const VkInstance& instance)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
			throw InitializationException("Failed to create debug messenger.");
	}

	static bool initialized = false;
	void init(u32 width, u32 height, bool releaseValidationLayers)
	{
		if (initialized)
			throw InitializationException("Renderer already initialized, cannot initialize again before destruction.");

		if (!SDL_Init(SDL_INIT_VIDEO))
			throw InitializationException("SDL3 failed to initialize.");

		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");

		enableValidationLayers |= releaseValidationLayers;
		std::vector<const char*> validationLayers{};
		if (enableValidationLayers)
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");

		VkInstance instance = createInstance(validationLayers);

		SDL_Window* sdlWindow{};
		if (sdlWindow = SDL_CreateWindow("Mysterious Game", width, height, SDL_WINDOW_VULKAN); sdlWindow == nullptr)
			throw ResourceCreationException("Failed to create SDL window");

		VkSurfaceKHR surface{};
		SDL_Vulkan_CreateSurface(sdlWindow, instance, nullptr, &surface);
		detail::context = new detail::Context(instance, surface, validationLayers);
		detail::window = new detail::Window(sdlWindow, surface);
		detail::basicRenderPass = new detail::RenderPass();
		detail::basicLightingPipeline = nullptr;
		detail::vertexBuffer = new detail::VertexBuffer();
		detail::indexBuffer = new detail::IndexBuffer();
		detail::uniformBuffers = new detail::UniformBuffer[MAX_FRAMES_IN_FLIGHT];
		scene = new Scene(2);

		initialized = true;
	}

	void addGraphicsPipeline(const std::vector<char>& vertCode, const std::vector<char>& fragCode)
	{
		detail::basicLightingPipeline = new detail::GraphicsPipeline(vertCode, fragCode);
	}

	void addTexture(const std::string& path)
	{
		textures.push_back(new Texture(path));
	}

	void addModel(const std::string& path)
	{
		Model* model = new Model(path);
		// add to vertex buffer
		{
			const std::vector<Vertex>& verts = model->verts;
			VkDeviceSize bufferSize = sizeof(verts[0]) * verts.size();
			
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMem;
			detail::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMem);

			void* data;
			vkMapMemory(detail::context->logicalDevice, stagingBufferMem, 0, bufferSize, 0, &data);
			memcpy(data, verts.data(), (size_t)bufferSize);
			vkUnmapMemory(detail::context->logicalDevice, stagingBufferMem);

			model->vertexBufferOffset = detail::vertexBuffer->offset;
			detail::copyBuffer(stagingBuffer, detail::vertexBuffer->buffer, bufferSize, detail::vertexBuffer->offset);
			detail::vertexBuffer->offset += bufferSize;

			vkDestroyBuffer(detail::context->logicalDevice, stagingBuffer, nullptr);
			vkFreeMemory(detail::context->logicalDevice, stagingBufferMem, nullptr);
		}
		// add to index buffer
		{
			const std::vector<u32>& indices = model->indices;
			VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
			
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMem;
			detail::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMem);

			void* data;
			vkMapMemory(detail::context->logicalDevice, stagingBufferMem, 0, bufferSize, 0, &data);
			memcpy(data, indices.data(), (size_t)bufferSize);
			vkUnmapMemory(detail::context->logicalDevice, stagingBufferMem);

			model->indexBufferOffset = detail::indexBuffer->offset;
			detail::copyBuffer(stagingBuffer, detail::indexBuffer->buffer, bufferSize, detail::indexBuffer->offset);
			detail::indexBuffer->offset += bufferSize;

			vkDestroyBuffer(detail::context->logicalDevice, stagingBuffer, nullptr);
			vkFreeMemory(detail::context->logicalDevice, stagingBufferMem, nullptr);
		}
		models.push_back(model);
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex, u32 currentFrame)
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

		for (size_t modelIndex = 0; modelIndex < models.size(); modelIndex++)
		{
			Model* model = models[modelIndex];
			VkDeviceSize offsets[] = { model->vertexBufferOffset };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			
			vkCmdBindIndexBuffer(commandBuffer, detail::indexBuffer->buffer, model->indexBufferOffset, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				detail::basicLightingPipeline->layout,
				detail::DescriptorSetType::PerModel, 1,
				&textures[modelIndex]->descriptorSet, 0, nullptr);

			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				detail::basicLightingPipeline->layout,
				detail::DescriptorSetType::PerObject, 1,
				&sceneData.objectDescriptorSets[modelIndex], 0, nullptr);
				
			vkCmdDrawIndexed(commandBuffer, SXI_TO_U32(model->indices.size()), 1, 0, 0, 0);	
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw InvalidArgumentException("Failed to record command buffer");
	}

	void render(const Time& time)
	{
		if (!initialized)
			throw InitializationException("Renderer has not been initialized");

		const detail::FrameContext* frameContext = detail::context->currentFrameContext();
		vkWaitForFences(detail::context->logicalDevice, 1, &frameContext->inFlightFence, VK_TRUE, UINT64_MAX);
		
		u32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(detail::context->logicalDevice, detail::window->swapchain->swapchain, UINT64_MAX, frameContext->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		
		vkResetFences(detail::context->logicalDevice, 1, &frameContext->inFlightFence);
		scene->run(time);
			
		vkResetCommandBuffer(frameContext->commandBuffer, 0);
		recordCommandBuffer(frameContext->commandBuffer, imageIndex, detail::context->currentFrame());

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

	void destroy()
	{
		if (!initialized)
			return;
		
		vkDeviceWaitIdle(detail::context->logicalDevice);
		
		for (Texture* tex : textures)
			delete tex;
		for (Model* model : models)
			delete model;
		delete detail::vertexBuffer;
		delete detail::indexBuffer;
		delete[] detail::uniformBuffers;
		delete scene;
		if (detail::basicLightingPipeline)
			delete detail::basicLightingPipeline;

		delete detail::basicRenderPass;
		delete detail::window;
		destroyDebugUtilsMessengerEXT(detail::context->instance, debugMessenger, nullptr);
		delete detail::context;

	    SDL_Quit();
		initialized = false;
	}
}