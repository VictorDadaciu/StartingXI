#pragma once

#include <Renderer2.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <array>
#include <unordered_map>

#include "SXICore/Types.h"

namespace sxi::renderer::detail
{
	constexpr u8 MAX_FRAMES_IN_FLIGHT = 2;

	struct FrameContext
	{
		VkCommandPool commandPool{};
		VkSemaphore imageAvailableSemaphore{};
		VkFence inFlightFence{};

		FrameContext(const VkDevice&, u8);
		~FrameContext();

	private:
		void createCommandPool(const VkDevice&, u8);
		void createSyncObjects(const VkDevice&);
	};

	struct PhysicalDevice
	{
		VkPhysicalDevice device{};
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceFeatures features{};
		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities{};
			std::vector<VkSurfaceFormatKHR> formats{};
			std::vector<VkPresentModeKHR> presentModes{};
		} swapchainSupport{};
		VkPhysicalDeviceMemoryProperties memProperties{};

		VkSampleCountFlagBits maxMSAASamples{};

		PhysicalDevice(const VkPhysicalDevice&, const VkSurfaceKHR&);

		bool suitable() const;
		bool supportsExtensions() const;
		int score() const;

	private:
		VkSampleCountFlagBits getMaxUsableSampleCount() const;
	};

	enum DescriptorSetType
	{
		PerFrame = 0,
		PerMaterial = 1,
		PerObject = 2,
		Count = 3
	};

	enum QueueFamilyIndex {
		GRAPHICS = 0,
		PRESENT = 1,
		COUNT = 2
	};

	extern struct Context
	{
		VkInstance instance{};
		std::vector<PhysicalDevice> physicalDevices;
		VkDevice logicalDevice{};
		VkQueue graphicsQueue{};
		VkQueue presentQueue{};
		VkQueue transferQueue{};
        std::array<std::pair<u8, u8>, QueueFamilyIndex::COUNT> queueFamiliesUsed{};
		VkDescriptorPool descriptorPool{};
		std::array<VkDescriptorSetLayout, DescriptorSetType::Count> descriptorSetLayouts{};

		std::array<FrameContext*, MAX_FRAMES_IN_FLIGHT> frameContexts{};

		Context(const VkInstance&, const VkSurfaceKHR&, const std::vector<const char*>&);
		~Context();

		inline const PhysicalDevice& currentPhysicalDevice() const { return physicalDevices[currentPhysicalDeviceIndex]; }
		inline const FrameContext* currentFrameContext() const { return frameContexts[currentFrame]; }

	private:
		void enumeratePhysicalDevices(const VkSurfaceKHR&);
		void chooseBestPhysicalDevice();
		void createLogicalDevice(const VkSurfaceKHR&, const std::vector<const char*>&);
		std::array<std::pair<u8, u8>, QueueFamilyIndex::COUNT> chooseQueueFamilyIndices(const VkSurfaceKHR&) const;
		void createDescriptorPool();
		void createDescriptorSetLayouts();

		inline void nextFrame() { currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; }

		size_t currentPhysicalDeviceIndex{};
		u8 currentFrame{};

		friend void sxi::renderer::render();
	} *context;
}