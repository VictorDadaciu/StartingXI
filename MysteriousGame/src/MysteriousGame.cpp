#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <vector>

#include "SXIRenderer/Renderer2.h"
#include "SXIRenderer/Window.h"
#include "SXIRenderer/Model.h"
#include "SXICore/File.h"
#include "SXICore/Timing.h"

const std::string MODELS_PATH = "../../MysteriousGame/models/";
const std::string TEXTURES_PATH = "../../MysteriousGame/textures/";
const std::string SHADERS_PATH = "../../MysteriousGame/shaders/";
const std::string SHADERS_GEN_PATH = "../../MysteriousGame/shaders/generated/";

static sxi::Texture texture(TEXTURES_PATH + "chair_basecolor.png");
static sxi::Model model(MODELS_PATH + "Rocking_Chair.obj", &texture);

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
		// if (!minimized)
		// 	renderer->render(time);

		time.refresh();
	}
}

int main(int argc, char* args[])
{
	sxi::renderer::init();
	//loop();
	sxi::renderer::destroy();
	return 0;
}