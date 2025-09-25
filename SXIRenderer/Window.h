#pragma once

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

namespace sxi
{
	class Window
	{
	public:
		Window();
		Window(int, int);
		~Window();

		void initialize();
		const VkSurfaceKHR surface(const VkInstance) const;
		void sizeInPixels(int& width, int& height) const { SDL_GetWindowSizeInPixels(window, &width, &height); }
		void close();
 
	private:
		int width{ 640 };
		int height{ 480 };
		bool initialized{ false };

		SDL_Window* window{ nullptr };
	};
}

