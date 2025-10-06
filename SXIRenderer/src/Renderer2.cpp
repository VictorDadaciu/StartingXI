#include "Renderer2.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <limits>
#include <set>

#include "SXICore/Types.h"
#include "SXICore/Exception.h"
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

namespace sxi
{
	const static uint8_t MAX_FRAMES_IN_FLIGHT = 2;
#ifdef NDEBUG
	static bool enableValidationLayers = false;
	const static std::vector<const char*> validationLayers(0);
#else
	static bool enableValidationLayers = true;
	const static std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#endif // DEBUG

	static const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	static bool checkValidationLayerSupport()
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
}

namespace sxi::renderer
{
	struct PhysicalDevice
	{
		PhysicalDevice() = default;
		PhysicalDevice(const VkPhysicalDevice& physicalDevice) : device(physicalDevice)
		{
			vkGetPhysicalDeviceProperties(device, &properties);
			vkGetPhysicalDeviceFeatures(device, &features);

			u32 queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
			queueFamilies.resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
		}

		bool suitable() const
		{
			if (!supportsExtensions())
				return false;

			if (!features.samplerAnisotropy)
				return false;

			// TODO
			// SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			// if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
			// 	return std::numeric_limits<int>::min();
            return true;
		}

        bool supportsExtensions() const
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

		int score() const
		{
			int score = 0;
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 100;
			else if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				return std::numeric_limits<int>::min();

			return score;
		}

		VkPhysicalDevice device{};
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceFeatures features{};
		std::vector<VkQueueFamilyProperties> queueFamilies{};
	};

	static struct Context
	{
		Context(std::vector<const char*>&& extensions)
		{
			createInstance(std::move(extensions));
			if (enableValidationLayers)
				setupDebugCallback();
			enumeratePhysicalDevices();
			chooseBestPhysicalDevice();
            createLogicalDevice();
		}

		~Context()
		{   
            vkDeviceWaitIdle(device);
			vkDestroyDevice(device, nullptr);

            if (enableValidationLayers)
			    destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

			vkDestroyInstance(instance, nullptr);
		}

		VkInstance instance{};
		std::vector<PhysicalDevice> physicalDevices{};
		size_t currentPhysicalDeviceIndex{};
		VkDevice device{};

	private:
		void createInstance(std::vector<const char*>&& extensions)
		{
			if (enableValidationLayers && !checkValidationLayerSupport())
				throw InitializationException("Vulkan validation layers not supported by installed version.");

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
				throw std::runtime_error("Failed to create Vulkan instance");
		}

		void setupDebugCallback() const
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			populateDebugMessengerCreateInfo(createInfo);

			if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
				throw InitializationException("Failed to create debug messenger.");
		}

		void enumeratePhysicalDevices()
		{
			u32 deviceCount;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0)
				throw InitializationException("No physical device found.");

			std::vector<VkPhysicalDevice> candidatePhysicalDevices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, candidatePhysicalDevices.data());
			for (VkPhysicalDevice physicalDevice : candidatePhysicalDevices)
			{
				PhysicalDevice candidate(physicalDevice);
				if (candidate.suitable())
					physicalDevices.push_back(std::move(candidate));
			}
			physicalDevices.shrink_to_fit();
			if (physicalDevices.size() == 0)
				throw InitializationException("No suitable physical device found.");
		}

		void chooseBestPhysicalDevice()
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

        void createLogicalDevice()
        {
            float queuePriority = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = 0;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

			VkPhysicalDeviceFeatures enabledFeatures{};
			enabledFeatures.samplerAnisotropy = VK_TRUE;
			enabledFeatures.sampleRateShading = VK_TRUE;

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = &queueCreateInfo;
			createInfo.queueCreateInfoCount = 1;
			createInfo.pEnabledFeatures = &enabledFeatures;
			createInfo.enabledExtensionCount = SXI_TO_U32(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = SXI_TO_U32(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else
			{
				createInfo.enabledLayerCount = 0;
			}

			if (vkCreateDevice(physicalDevices[currentPhysicalDeviceIndex].device, &createInfo, nullptr, &device) != VK_SUCCESS)
				throw InitializationException("Failed to create logical device.");

            VkQueue queue{};
			vkGetDeviceQueue(device, 0, 0, &queue);
        }
	} *context{};

	static bool initialized = false;
	void init(bool releaseValidationLayers)
	{
		if (initialized)
			throw InitializationException("Renderer already initialized, cannot initialize again before destruction.");

		if (!SDL_Init(SDL_INIT_VIDEO))
			throw InitializationException("SDL3 failed to initialize.");

		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");

		u32 extensionsCount{};
		const char* const * SDL_extensions = SDL_Vulkan_GetInstanceExtensions(&extensionsCount);
		std::vector<const char*> extensions(SDL_extensions, SDL_extensions + extensionsCount);
		if (!enableValidationLayers)
			enableValidationLayers = releaseValidationLayers;
		if (enableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		context = new Context(std::move(extensions));

		initialized = true;
	}

	void destroy()
	{
		if (!initialized)
			return;

		delete context;

	    SDL_Quit();
		initialized = false;
	}
}