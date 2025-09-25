#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "Vec.h"

namespace sxi
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		inline bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats{};
		std::vector<VkPresentModeKHR> presentModes{};
	};

	class Window;
	class Renderer
	{
	public:
		Renderer(std::vector<const char*>&& extensions);
	
		void initialize(Window*, std::vector<char>&&, std::vector<char>&&);
		inline const VkInstance vkInstance() const { return instance; }
		void render();
		void cleanup() const;
	
		~Renderer() = default;
	
		Renderer(const Renderer&) = delete;
		void operator=(const Renderer&) = delete;
	
	private:
		void createInstance(std::vector<const char*>&&);
		void setupDebugCallback() const;
		void choosePhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipeline(std::vector<char>&&, std::vector<char>&&);
		void createFrameBuffers();
		void createCommandPool();
		void createVertexBuffer();
		void createCommandBuffer();
		void createSyncObjects();

		void recordCommandBuffer(VkCommandBuffer, uint32_t);
		void recreateSwapChain();
		void cleanupSwapChain() const;

		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device) const;
		int physicalDeviceScore(const VkPhysicalDevice device) const;
		SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice) const;
		VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&) const;
		VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>&) const;
		VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR&) const;
		VkShaderModule createShaderModule(std::vector<char>&&) const;
		uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags) const;
	
		Window* window = nullptr;

		VkInstance instance{};
		VkSurfaceKHR surface{};
		VkPhysicalDevice physicalDevice{};
		VkDevice logicalDevice{};
		VkSwapchainKHR swapChain{};
		std::vector<VkImage> swapChainImages{};
		std::vector<VkImageView> swapChainImageViews{};
		VkFormat swapChainImageFormat{};
		VkExtent2D swapChainExtent{};
		VkRenderPass renderPass{};
		VkPipelineLayout pipelineLayout{};
		VkPipeline graphicsPipeline{};
		std::vector<VkFramebuffer> swapChainFrameBuffers{};
		VkCommandPool commandPool{};
		VkBuffer vertexBuffer{};
		VkDeviceMemory vertexBufferMem{};
		std::vector<VkCommandBuffer> commandBuffers{};
		std::vector<VkSemaphore> imageAvailableSemaphores{};
		std::vector<VkSemaphore> renderFinishedSemaphores{};
		std::vector<VkFence> inFlightFences{};

		VkQueue graphicsQueue{};
		VkQueue presentQueue{};

		VkPhysicalDeviceFeatures deviceFeatures{};
		bool initialized = false;
	};
}

