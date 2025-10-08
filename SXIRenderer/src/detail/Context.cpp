#include "detail/Context.h"

#include <string>
#include <set>
#include <unordered_map>
#include <limits>
#include <array>
#include <iostream>

#include "SXICore/Exception.h"

namespace sxi::renderer::detail
{
    Context* context{};

    static const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface) : device(physicalDevice)
	{
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainSupport.capabilities);

		u32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			swapchainSupport.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapchainSupport.formats.data());
		}

		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			swapchainSupport.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapchainSupport.presentModes.data());
		}
	}

	bool PhysicalDevice::suitable() const
	{
		if (!supportsExtensions())
			return false;

		if (!features.samplerAnisotropy)
			return false;

		if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
			return false;
		return true;
	}

	bool PhysicalDevice::supportsExtensions() const
	{
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		return requiredExtensions.empty();
	}

	int PhysicalDevice::score() const
	{
		int score = 0;
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 100;
		else if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			return std::numeric_limits<int>::min();

		return score;
	}

	Context::Context(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& layers) : instance(instance)
	{
		enumeratePhysicalDevices(surface);
		chooseBestPhysicalDevice();
		createLogicalDevice(surface, layers);
	}

	Context::~Context()
	{
		vkDestroyDevice(logicalDevice, nullptr);

		vkDestroyInstance(instance, nullptr);
	}

	void Context::enumeratePhysicalDevices(const VkSurfaceKHR& surface)
	{
		u32 deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw InitializationException("No physical device found.");

		std::vector<VkPhysicalDevice> candidatePhysicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, candidatePhysicalDevices.data());
		for (VkPhysicalDevice physicalDevice : candidatePhysicalDevices)
		{
			PhysicalDevice candidate(physicalDevice, surface);
			if (candidate.suitable())
				physicalDevices.push_back(std::move(candidate));
		}
		physicalDevices.shrink_to_fit();
		if (physicalDevices.size() == 0)
			throw InitializationException("No suitable physical device found.");
	}

	void Context::chooseBestPhysicalDevice()
	{
		int bestScore = -1;
		for (size_t i = 0; i < physicalDevices.size(); ++i)
		{
			int score = physicalDevices[i].score();
			if (score > bestScore)
			{
				bestScore = score;
				currentPhysicalDeviceIndex = i;
				// TODO msaaSamples = getMaxUsableSampleCount();
			}
		}
	}

	void Context::createLogicalDevice(const VkSurfaceKHR& surface, const std::vector<const char*>& layers)
	{
		// pair -> first: queue family index, second: queue index within queue family
		std::array<std::pair<u8, u8>, QueueFamilyInternalIdx::COUNT> queueFamilyIndices = chooseQueueFamilyIndices(surface);
		
        std::unordered_map<u8, u8> indicesCoalesced{};
        for (const std::pair<u8, u8>& queueRequest : queueFamilyIndices)
            if (indicesCoalesced.find(queueRequest.first) != indicesCoalesced.end())
                indicesCoalesced[queueRequest.first]++;
            else
                indicesCoalesced[queueRequest.first] = 1;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		for (const auto& it : indicesCoalesced)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = it.first;
			queueCreateInfo.queueCount = it.second;
            std::vector<float> priorities(it.second, 1.f);
			queueCreateInfo.pQueuePriorities = priorities.data();
            queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures enabledFeatures{};
		enabledFeatures.samplerAnisotropy = VK_TRUE;
		enabledFeatures.sampleRateShading = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = SXI_TO_U32(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &enabledFeatures;
		createInfo.enabledExtensionCount = SXI_TO_U32(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (layers.size() > 0)
		{
			createInfo.enabledLayerCount = SXI_TO_U32(layers.size());
			createInfo.ppEnabledLayerNames = layers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(currentPhysicalDevice().device, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create logical device.");

		vkGetDeviceQueue(logicalDevice, queueFamilyIndices[GRAPHICS].first, queueFamilyIndices[GRAPHICS].second, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices[PRESENT].first, queueFamilyIndices[PRESENT].second, &presentQueue);
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices[TRANSFER].first, queueFamilyIndices[TRANSFER].second, &transferQueue);
        queueFamilyIndicesUsed[GRAPHICS] = queueFamilyIndices[GRAPHICS].first;
        queueFamilyIndicesUsed[PRESENT] = queueFamilyIndices[PRESENT].first;
	}

	std::array<std::pair<u8, u8>, Context::QueueFamilyInternalIdx::COUNT> 
		Context::chooseQueueFamilyIndices(const VkSurfaceKHR& surface) const
	{
		std::vector<VkQueueFamilyProperties> queueFamilies{};
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(currentPhysicalDevice().device, &queueFamilyCount, nullptr);
		queueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(currentPhysicalDevice().device, &queueFamilyCount, queueFamilies.data());

		std::array<std::pair<u8, u8>, QueueFamilyInternalIdx::COUNT> queueFamilyIndices{};
		for (size_t i = 0; i < queueFamilies.size(); ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueFamilyIndices[GRAPHICS] = std::pair<u8, u8>(SXI_TO_U8(i), 0);
				break;
			}
		}

		bool transferFound = false;
		for (size_t i = 0; i < queueFamilies.size(); ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
			{
				queueFamilyIndices[TRANSFER] = std::pair<u8, u8>(SXI_TO_U8(i), 0);
				transferFound = true;
				break;
			}
		}

		if (!transferFound)
		{
			for (size_t i = 0; i < queueFamilies.size(); ++i)
			{
				if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					queueFamilyIndices[TRANSFER] = std::pair<u8, u8>(SXI_TO_U8(i), 0);
					if (queueFamilyIndices[TRANSFER].first != queueFamilyIndices[GRAPHICS].first)
						break;
					
					if (queueFamilies[i].queueCount > 2)
					{
						queueFamilyIndices[TRANSFER].second = 2;
						break;
					}
					
					if (queueFamilies[i].queueCount > 1)
					{
						queueFamilyIndices[TRANSFER].second = 1;
						break;
					}
				}
			}
		}

		bool presentFound = false;
		for (size_t i = 0; i < queueFamilies.size(); ++i)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(currentPhysicalDevice().device, i, surface, &presentSupport);
			if (presentSupport)
			{
				if (queueFamilyIndices[GRAPHICS].first != queueFamilyIndices[PRESENT].first && queueFamilyIndices[TRANSFER].first != queueFamilyIndices[PRESENT].first)
				{
					queueFamilyIndices[PRESENT] = std::pair<u8, u8>(SXI_TO_U8(i), 0);
					presentFound = true;
					break;
				}
			}
		}

		if (!presentFound)
		{
			for (size_t i = 0; i < queueFamilies.size(); ++i)
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(currentPhysicalDevice().device, i, surface, &presentSupport);
				if (presentSupport)
				{
					queueFamilyIndices[PRESENT] = std::pair<u8, u8>(SXI_TO_U8(i), 0);
					if (queueFamilyIndices[GRAPHICS].first == queueFamilyIndices[PRESENT].first)
					{
						queueFamilyIndices[PRESENT].second = queueFamilies[i].queueCount > 1;
						break;
					}

					if (queueFamilyIndices[TRANSFER].first == queueFamilyIndices[PRESENT].first && queueFamilies[i].queueCount > 1)
					{
						queueFamilyIndices[PRESENT].second = 1;
						break;
					}
				}
			}
		}
		return queueFamilyIndices;
	}
}