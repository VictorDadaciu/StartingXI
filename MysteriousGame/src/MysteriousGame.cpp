#include "ECSSettings.h"
#include "SXICore/ECS/Manager.h"
#include "SXICore/File.h"
#include "SXICore/JobScheduler.h"
#include "SXICore/Timing.h"
#include "SXIRenderer/Renderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <string>

const std::string MODELS_PATH = "../../MysteriousGame/models/";
const std::string TEXTURES_PATH = "../../MysteriousGame/textures/";
const std::string SHADERS_PATH = "../../MysteriousGame/shaders/";
const std::string SHADERS_GEN_PATH = "../../MysteriousGame/shaders/generated/";

static sxi::ecs::Manager<ECSSettings> mgr;
static sxi::JobScheduler scheduler;
static sxi::Time timer{};

class TestFunc
{
public:
    TestFunc() = default;

    void operator()(size_t x, size_t y)
    {
        inverse ? (std::cout << y << " " << x << "\n") : (std::cout << x << " " << y << "\n");
    }

    static void flip() { inverse = !inverse; }

private:
    inline static bool inverse{};
};

static void loop()

{
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

        TestFunc::flip();

        scheduler.run();

        mgr.refresh();
        scheduler.refresh();
        timer.refresh();
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

    sxi::ScheduleId idPos = scheduler.schedule(
        [&](size_t start, size_t end)
        {
            mgr.forEntitiesMatching<MoveSignature>(
                [](auto&, auto& posComponent)
                {
                    static sxi::TimePoint start = timer.time;
                    posComponent.pos.y = sinf(sxi::Time::elapsed(start, timer.time));
                },
                start,
                end);
        },
        {SXI_CHECKPOINT_BEGIN},
        []()
        {
            return sxi::WorkSize{.chunkSize = 1, .numChunks = 2};
        });

    sxi::ScheduleId idRot = scheduler.schedule(
        [&](size_t start, size_t end)
        {
            mgr.forEntitiesMatching<RotateSignature>(
                [](auto&, auto& rotComponent)
                {
                    rotComponent.rot.y += 0.1 * timer.dt;
                },
                start,
                end);
        },
        {SXI_CHECKPOINT_BEGIN},
        []()
        {
            return sxi::WorkSize{.chunkSize = 1, .numChunks = 2};
        });

    scheduler.schedule(TestFunc(), {SXI_CHECKPOINT_BEGIN});

    scheduler.schedule(
        [&](size_t start, size_t end)
        {
            sxi::renderer::render(mgr, timer);
        },
        {idPos, idRot});
    scheduler.refresh();

    loop();
    sxi::renderer::destroy();

    return 0;
}