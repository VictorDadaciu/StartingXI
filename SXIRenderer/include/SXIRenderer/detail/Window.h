#pragma once

#include "Context.h"

#include <SDL3/SDL.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace sxi::renderer::detail
{
struct Swapchain
{
    VkSwapchainKHR swapchain{};
    std::vector<VkImage> images{};
    std::vector<VkImageView> imageViews{};
    VkSurfaceFormatKHR surfaceFormat{};
    VkExtent2D extent{};
    VkPresentModeKHR presentMode{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkSemaphore> timelineSemaphores{};

    Swapchain(SDL_Window*, const VkSurfaceKHR&);
    ~Swapchain();

private:
    void populateProperties(SDL_Window*, const PhysicalDevice::SwapchainSupportDetails&);
};

extern struct Window
{
    SDL_Window* sdlWindow{};
    VkSurfaceKHR surface{};
    Swapchain* swapchain{};

    Window(SDL_Window*, const VkSurfaceKHR&);
    ~Window();

    void recreateSwapchain();

    inline void sizeInPixels(int& width, int& height) const { SDL_GetWindowSizeInPixels(sdlWindow, &width, &height); }

private:
}* window;
} // namespace sxi::renderer::detail