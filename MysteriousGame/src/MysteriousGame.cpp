#include "ECSSettings.h"
#include "SXICore/ECS/Manager.h"
#include "SXICore/File.h"
#include "SXICore/Timing.h"
#include "SXIRenderer/Renderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <string>

const std::string MODELS_PATH = "../../MysteriousGame/models/";
const std::string TEXTURES_PATH = "../../MysteriousGame/textures/";
const std::string SHADERS_PATH = "../../MysteriousGame/shaders/";
const std::string SHADERS_GEN_PATH = "../../MysteriousGame/shaders/generated/";

static sxi::ecs::Manager<ECSSettings> mgr;

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

        mgr.forEntitiesMatching<MoveSignature>(
            [&time](auto&, auto& posComponent)
            {
                static sxi::TimePoint start = time.time;
                posComponent.pos.y = sinf(sxi::Time::elapsed(start, time.time));
            });

        mgr.forEntitiesMatching<RotateSignature>(
            [&time](auto&, auto& rotComponent)
            {
                rotComponent.rot.y += 0.1 * time.dt;
            });

        sxi::renderer::render(mgr, time);
        mgr.refresh();
        time.refresh();
    }
}

int main(int argc, char* args[])
{
    sxi::renderer::init(1600, 900);
    sxi::renderer::addGraphicsPipeline(sxi::file::readFileAsBytes(SHADERS_GEN_PATH + "basic_lighting.vert.spv"),
                                       sxi::file::readFileAsBytes(SHADERS_GEN_PATH + "basic_lighting.frag.spv"));

    {
        sxi::ecs::EntityIndex<Object> ent = mgr.createEntity<Object>();
        sxi::ecs::RenderComponent& render = mgr.component<sxi::ecs::RenderComponent>(ent);
        render.mdl = sxi::renderer::addModel(MODELS_PATH + "Coffee_Table.obj");
        render.tex = sxi::renderer::addTexture(TEXTURES_PATH + "table_basecolor.png");
        sxi::ecs::PositionComponent& pos = mgr.component<sxi::ecs::PositionComponent>(ent);
        pos.pos = glm::vec3(20, 0, 20);
    }
    {
        sxi::ecs::EntityIndex<Object> ent = mgr.createEntity<Object>();
        sxi::ecs::RenderComponent& render = mgr.component<sxi::ecs::RenderComponent>(ent);
        render.mdl = sxi::renderer::addModel(MODELS_PATH + "Rocking_Chair.obj");
        render.tex = sxi::renderer::addTexture(TEXTURES_PATH + "chair_basecolor.png");
        sxi::ecs::PositionComponent& pos = mgr.component<sxi::ecs::PositionComponent>(ent);
        pos.pos = glm::vec3(-20, 0, 20);
    }
    mgr.createEntity<sxi::ecs::Light>();
    mgr.refresh();

    loop();
    sxi::renderer::destroy();

    return 0;
}