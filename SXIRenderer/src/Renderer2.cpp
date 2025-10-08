#include "Renderer2.h"
#include "detail/Context.h"
#include "detail/Window2.h"

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

		initialized = true;
	}

	void destroy()
	{
		if (!initialized)
			return;
		
		vkDeviceWaitIdle(detail::context->logicalDevice);
		
		delete detail::window;
		destroyDebugUtilsMessengerEXT(detail::context->instance, debugMessenger, nullptr);
		delete detail::context;

	    SDL_Quit();
		initialized = false;
	}
}