#include "detail/Context.h"

#include "detail/Utils.h"

#include <string>
#include <set>
#include <limits>
#include <array>
#include <iostream>

#include "SXICore/Types.h"
#include "SXICore/Exception.h"

namespace sxi::renderer::detail
{
    Context* context{};

    static const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

	FrameContext::FrameContext(const VkDevice& logicalDevice, u8 graphicsQueueFamilyIndex)
	{
		createCommandPool(logicalDevice, graphicsQueueFamilyIndex);
		createCommandBuffer(logicalDevice);
		createSyncObjects(logicalDevice);
	}

	FrameContext::~FrameContext()
	{
		vkDestroySemaphore(context->logicalDevice, imageAvailableSemaphore, nullptr);
		vkDestroyFence(context->logicalDevice, inFlightFence, nullptr);

		vkDestroyCommandPool(context->logicalDevice, commandPool, nullptr);
	}

	void FrameContext::createCommandPool(const VkDevice& logicalDevice, u8 graphicsQueueFamilyIndex)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

		if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create command pool");
	}

	void FrameContext::createCommandBuffer(const VkDevice& logicalDevice)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
			throw ResourceCreationException("Failed to allocate command buffers");
	}

	void FrameContext::createSyncObjects(const VkDevice& logicalDevice)
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create semaphore");

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create fence");

	}

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

		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

		maxMSAASamples = getMaxUsableSampleCount();
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

	VkSampleCountFlagBits PhysicalDevice::getMaxUsableSampleCount() const
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	Context::Context(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& layers) : instance(instance)
	{
		enumeratePhysicalDevices(surface);
		chooseBestPhysicalDevice();
		createLogicalDevice(surface, layers);
		createDescriptorPool();
		createDescriptorSetLayouts();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
			frameContexts[i] = new FrameContext(logicalDevice, queueFamiliesUsed[GRAPHICS].first);
	}

	Context::~Context()
	{
		for (FrameContext* frameContext : frameContexts)
			delete frameContext;

		for (const VkDescriptorSetLayout& layout : descriptorSetLayouts)
			vkDestroyDescriptorSetLayout(logicalDevice, layout, nullptr);

		vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);

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
		queueFamiliesUsed = chooseQueueFamilyIndices(surface);
		
		std::unordered_map<u8, u8> queueFamilyIndicesCondensed{};
        for (const std::pair<u8, u8>& queueRequest : queueFamiliesUsed)
            if (queueFamilyIndicesCondensed.find(queueRequest.first) != queueFamilyIndicesCondensed.end())
                queueFamilyIndicesCondensed[queueRequest.first]++;
            else
                queueFamilyIndicesCondensed[queueRequest.first] = 1;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		for (const auto& it : queueFamilyIndicesCondensed)
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

		vkGetDeviceQueue(logicalDevice, queueFamiliesUsed[GRAPHICS].first, queueFamiliesUsed[GRAPHICS].second, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, queueFamiliesUsed[PRESENT].first, queueFamiliesUsed[PRESENT].second, &presentQueue);
	}

	std::array<std::pair<u8, u8>, QueueFamilyIndex::COUNT> 
		Context::chooseQueueFamilyIndices(const VkSurfaceKHR& surface) const
	{
		std::vector<VkQueueFamilyProperties> queueFamilies{};
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(currentPhysicalDevice().device, &queueFamilyCount, nullptr);
		queueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(currentPhysicalDevice().device, &queueFamilyCount, queueFamilies.data());

		std::array<std::pair<u8, u8>, QueueFamilyIndex::COUNT> queueFamilyIndices{};
		for (size_t i = 0; i < queueFamilies.size(); ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueFamilyIndices[GRAPHICS] = std::pair<u8, u8>(SXI_TO_U8(i), 0);
				break;
			}
		}

		bool presentFound = false;
		for (size_t i = 0; i < queueFamilies.size(); ++i)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(currentPhysicalDevice().device, i, surface, &presentSupport);
			if (presentSupport)
			{
				if (queueFamilyIndices[GRAPHICS].first != queueFamilyIndices[PRESENT].first)
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
				}
			}
		}
		return queueFamilyIndices;
	}

	void Context::createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1000;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 2;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = SXI_TO_U32(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1000;

		if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create descriptor pool");
	}

	void Context::createDescriptorSetLayouts()
	{
		// per frame
		{
			VkDescriptorSetLayoutBinding matricesLayoutBinding{};
			matricesLayoutBinding.binding = 0;
			matricesLayoutBinding.descriptorCount = 1;
			matricesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			matricesLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutBinding lightLayoutBinding{};
			lightLayoutBinding.binding = 1;
			lightLayoutBinding.descriptorCount = 1;
			lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			
			std::array<VkDescriptorSetLayoutBinding, 2> bindings = {matricesLayoutBinding, lightLayoutBinding};
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = SXI_TO_U32(bindings.size());
			layoutInfo.pBindings = bindings.data();
			
			if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayouts[DescriptorSetType::PerFrame]) != VK_SUCCESS)
				throw ResourceCreationException("Failed to create descriptor set layout");
		}
		// per model
		{
			VkDescriptorSetLayoutBinding albedoLayoutBinding{};
			albedoLayoutBinding.binding = 0;
			albedoLayoutBinding.descriptorCount = 1;
			albedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			albedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &albedoLayoutBinding;
			
			if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayouts[DescriptorSetType::PerModel]) != VK_SUCCESS)
				throw ResourceCreationException("Failed to create descriptor set layout");
		}
		// per object
		{
			VkDescriptorSetLayoutBinding matricesLayoutBinding{};
			matricesLayoutBinding.binding = 0;
			matricesLayoutBinding.descriptorCount = 1;
			matricesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			matricesLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &matricesLayoutBinding;
			
			if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayouts[DescriptorSetType::PerObject]) != VK_SUCCESS)
				throw ResourceCreationException("Failed to create descriptor set layout");
		}
	}
}