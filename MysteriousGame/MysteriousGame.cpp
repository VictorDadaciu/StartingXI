#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <vector>

#include "Renderer.h"
#include "Window.h"
#include "File.h"
#include "Timing.h"

static sxi::Window* window{};
static sxi::Renderer* renderer{};

static void initializeSDL()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
		throw std::exception("Failed to initialize SDL3");
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");
	window = new sxi::Window();
	window->initialize();
}

static void initializeRenderer()
{
	Uint32 extensionsCount{ 0 };
	const char* const * SDL_extensions = SDL_Vulkan_GetInstanceExtensions(&extensionsCount);
	std::vector<const char*> extensions(SDL_extensions, SDL_extensions + extensionsCount);
	renderer = new sxi::Renderer(std::move(extensions));
	int wInPixels, hInPixels;
	window->sizeInPixels(wInPixels, hInPixels);

	renderer->initialize(
		window,
		sxi::file::readFileAsBytes("C:/code/StartingXI/MysteriousGame/shaders/generated/triangle_vert.spv"),
		sxi::file::readFileAsBytes("C:/code/StartingXI/MysteriousGame/shaders/generated/triangle_frag.spv")
	);
}

static void initialize()
{
	initializeSDL();
	initializeRenderer();
}

static void loop()
{
	sxi::Time time{};
	SDL_Event e;
	SDL_zero(e);
	bool minimized = false;
	bool running = true;
	while (running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
				minimized = true;

			if (e.type == SDL_EVENT_WINDOW_RESTORED)
				minimized = false;

			if (e.type == SDL_EVENT_QUIT)
				return;

			if (e.type == SDL_EVENT_KEY_DOWN)
			{
				if (e.key.key == SDLK_ESCAPE)
					return;
			}
		}
		if (!minimized)
			renderer->render(time);

		time.refresh();
	}
}

static void close()
{
	renderer->cleanup();
	window->close();
	delete renderer;
	delete window;
	SDL_Quit();
}

int main(int argc, char* args[])
{
	initialize();
	loop();
	close();
	return 0;
}