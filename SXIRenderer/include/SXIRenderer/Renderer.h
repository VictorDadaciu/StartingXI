#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "SXIMath/Vec.h"
#include "SXIMath/Mat.h"
#include "SXICore/Timing.h"
#include "SXICore/Types.h"

namespace sxi
{
	struct PushConstants {
		alignas(16) glm::vec3 lightPos;
	};

	struct QueueFamilyIndices
	{
		std::optional<u32> graphicsFamily;
		std::optional<u32> presentFamily;

		inline bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats{};
		std::vector<VkPresentModeKHR> presentModes{};
	};
	
	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class Model;
	class Window;
	class Renderer
	{
	public:
		Renderer(std::vector<const char*>&& extensions);
	
		void initialize(Window*, std::vector<char>&&, std::vector<char>&&, Model*);
		inline const VkInstance vkInstance() const { return instance; }
		void render(const Time&);
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
		void createDescriptorSetLayout();
		void createGraphicsPipeline(std::vector<char>&&, std::vector<char>&&);
		void createFrameBuffers();
		void createCommandPool();
		void createColorResources();
		void createDepthResources();
		void createTextureImage();
		void createTextureImageView();
		void createTextureImageSampler();
		void createVertexBuffer();
		void createIndexBuffer();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffer();
		void createSyncObjects();

		void updateUniformBuffers(const Time&, u32);
		void recordCommandBuffer(VkCommandBuffer, u32, u32);
		void recreateSwapChain();
		void cleanupSwapChain() const;

		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice) const;
		int physicalDeviceScore(const VkPhysicalDevice) const;
		SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice) const;
		VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&) const;
		VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>&) const;
		VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR&) const;
		VkShaderModule createShaderModule(std::vector<char>&&) const;
		u32 findMemoryType(u32, VkMemoryPropertyFlags) const;
		void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
		void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize) const;
		void createImage(u32, u32, u32, VkSampleCountFlagBits, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
		VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags, u32) const;
		void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout, u32);
		void copyBufferToImage(VkBuffer, VkImage, u32, u32);
		VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags) const;
		VkFormat findDepthFormat() const;
		void generateMipmaps(VkImage, VkFormat, i32, i32, u32);
		VkSampleCountFlagBits getMaxUsableSampleCount() const;

		VkCommandBuffer beginSingleTimeCommands() const;
		void endSingleTimeCommands(VkCommandBuffer) const;
	
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
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMem;
		std::vector<void*> uniformBuffersMapped;
		VkPipeline graphicsPipeline{};
		std::vector<VkFramebuffer> swapChainFrameBuffers{};
		VkCommandPool commandPool{};
		VkImage textureImage;
		VkDeviceMemory textureImageMem;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkImage colorImage;
		VkDeviceMemory colorImageMem;
		VkImageView colorImageView;
		VkImage depthImage;
		VkDeviceMemory depthImageMem;
		VkImageView depthImageView;
		VkBuffer vertexBuffer{};
		VkDeviceMemory vertexBufferMem{};
		VkBuffer indexBuffer{};
		VkDeviceMemory indexBufferMem{};
		std::vector<VkCommandBuffer> commandBuffers{};
		std::vector<VkSemaphore> imageAvailableSemaphores{};
		std::vector<VkSemaphore> renderFinishedSemaphores{};
		std::vector<VkFence> inFlightFences{};
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkQueue graphicsQueue{};
		VkQueue presentQueue{};

		Model* model;
		std::array<glm::vec3, 2> lightPos{};

		VkPhysicalDeviceFeatures deviceFeatures{};
		bool initialized = false;
	};
}

