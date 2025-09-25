#include "Window.h"
#include <SDL3/SDL_vulkan.h>
#include <exception>

namespace sxi
{
	Window::Window() {}

	Window::Window(int width, int height) : width(width), height(height) {}

	void Window::initialize()
	{
		if(window = SDL_CreateWindow("Mysterious Game", width, height, SDL_WINDOW_VULKAN); window == nullptr)
			throw std::exception("Failed to create window");

		initialized = true;
	}

	const VkSurfaceKHR Window::surface(const VkInstance instance) const
	{
		VkSurfaceKHR surface{};
		SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
		return surface;
	}

	void Window::close()
	{
		if (!initialized)
		{
			SDL_Log("WARNING: Trying to close unitialized window.\n");
			return;
		}

		SDL_DestroyWindow(window);
		window = nullptr;
		initialized = false;
	}

	Window::~Window()
	{
		if (initialized)
			SDL_Log("WARNING: Window object fell out of scope before being explicitly closed.\n");
	}
}