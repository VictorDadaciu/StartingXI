#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <array>
#include <unordered_map>

#include "SXICore/Types.h"

namespace sxi::renderer::detail
{
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

		PhysicalDevice(const VkPhysicalDevice&, const VkSurfaceKHR&);

		bool suitable() const;
		bool supportsExtensions() const;
		int score() const;
	};

	extern struct Context
	{
		enum QueueFamilyInternalIdx {
			GRAPHICS = 0,
			PRESENT = 1,
			TRANSFER = 2,
			COUNT = 3
		};

		VkInstance instance{};
		std::vector<PhysicalDevice> physicalDevices;
		size_t currentPhysicalDeviceIndex{};
		VkDevice logicalDevice{};
		VkQueue graphicsQueue{};
		VkQueue presentQueue{};
		VkQueue transferQueue{};
        std::unordered_map<u8, u8> queueFamiliesUsed{};

		Context(const VkInstance&, const VkSurfaceKHR&, const std::vector<const char*>&);
		~Context();

		inline const PhysicalDevice& currentPhysicalDevice() const { return physicalDevices[currentPhysicalDeviceIndex]; }

	private:
		void enumeratePhysicalDevices(const VkSurfaceKHR&);
		void chooseBestPhysicalDevice();
		void createLogicalDevice(const VkSurfaceKHR&, const std::vector<const char*>&);
		std::array<std::pair<u8, u8>, QueueFamilyInternalIdx::COUNT> chooseQueueFamilyIndices(const VkSurfaceKHR&) const;
	} *context;
}