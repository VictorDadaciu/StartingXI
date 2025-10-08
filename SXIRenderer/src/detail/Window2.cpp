#include "detail/Window2.h"
#include "detail/Context.h"

#include "SDL3/SDL_vulkan.h"

#include <limits>
#include <algorithm>
#include <iostream>
#include <set>

#include "SXICore/Exception.h"

namespace sxi::renderer::detail
{
    Window* window{};

    Window::Window(SDL_Window* sdlWindow, const VkSurfaceKHR& surface) : sdlWindow(sdlWindow), surface(surface), swapchain(new Swapchain(sdlWindow, surface)) {}

	Window::~Window()
	{
        delete swapchain;

		vkDestroySurfaceKHR(context->instance, surface, nullptr);
		SDL_DestroyWindow(sdlWindow);
	}

    void Window::recreateSwapchain()
    {
        delete swapchain;
        swapchain = new Swapchain(sdlWindow, surface);
    }    

    Swapchain::Swapchain(SDL_Window* sdlWindow, const VkSurfaceKHR& surface)
    {
        const PhysicalDevice::SwapchainSupportDetails& supportDetails = context->currentPhysicalDevice().swapchainSupport;
        populateProperties(sdlWindow, supportDetails);

		u32 imageCount = supportDetails.capabilities.minImageCount + 1;
		if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
			imageCount = supportDetails.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		std::vector<u32> queueFamiliesUsed{};
		for (const auto& queueFamilyUsed : context->queueFamiliesUsed)
			queueFamiliesUsed.push_back(queueFamilyUsed.first);

		if (context->queueFamiliesUsed.size() > 1)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = SXI_TO_U32(queueFamiliesUsed.size());
			createInfo.pQueueFamilyIndices = queueFamiliesUsed.data();
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		createInfo.preTransform = supportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(context->logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create swap chain");

		vkGetSwapchainImagesKHR(context->logicalDevice, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(context->logicalDevice, swapchain, &imageCount, images.data());
    }

    Swapchain::~Swapchain()
    {
        vkDeviceWaitIdle(context->logicalDevice);

		for (VkImageView imageView : imageViews)
			vkDestroyImageView(context->logicalDevice, imageView, nullptr);

		vkDestroySwapchainKHR(context->logicalDevice, swapchain, nullptr);
    }

    void Swapchain::populateProperties(SDL_Window* sdlWindow, const PhysicalDevice::SwapchainSupportDetails& supportDetails)
	{
        surfaceFormat = supportDetails.formats[0];
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
        extent = supportDetails.capabilities.currentExtent;
		for (const auto& availableFormat : supportDetails.formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = availableFormat;
                break;
            }
        }

        for (const auto& availablePresentMode : supportDetails.presentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = availablePresentMode;
                break;
            }
        }

        if (supportDetails.capabilities.currentExtent.width == std::numeric_limits<u32>::max())
		{
			int width, height;
			SDL_GetWindowSizeInPixels(sdlWindow, &width, &height);

			extent = {
				SXI_TO_U32(width),
				SXI_TO_U32(height)
			};

			extent.width = std::clamp(extent.width, supportDetails.capabilities.minImageExtent.width, supportDetails.capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, supportDetails.capabilities.minImageExtent.height, supportDetails.capabilities.maxImageExtent.height);
		}
	}
}
