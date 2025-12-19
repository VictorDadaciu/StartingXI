#include "ECSSettings.h"
#include "SXICore/ECS/Manager.h"
#include "SXICore/File.h"
#include "SXICore/JobScheduler.h"
#include "SXICore/Timing.h"
#include "SXICore/Types.h"
#include "SXICore/detail/Task.h"
#include "SXICore/logger/Logger.h"
#include "SXIRenderer/ECSConfig.h"
#include "SXIRenderer/Renderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <fstream>
#include <string>

const std::string BASE_PATH = "/home/victordadaciu/workspace/StartingXI/MysteriousGame/";
const std::string MODELS_PATH = BASE_PATH + "models/";
const std::string TEXTURES_PATH = BASE_PATH + "textures/";
const std::string SHADERS_PATH = BASE_PATH + "shaders/";
const std::string SHADERS_GEN_PATH = SHADERS_PATH + "generated/";

using namespace sxi::types;
constexpr u32 OBJ_SIDE = 70;

static sxi::ecs::Manager<ECSSettings> mgr;
static sxi::JobScheduler* scheduler{};
static sxi::Time timer{};
static sxi::Time renderTimer{};
static float renderTime{};
static sxi::Time localTimer{};
static float schedulingTime{};

void printStats(const sxi::TimePoint& timeAtStart, size_t frames)
{
    sxi::TimePoint timeAtEnd = timer.time;
    float timeActive = sxi::Time::elapsed(timeAtStart, timeAtEnd);
    float msPerSchedule = 1000.f * schedulingTime / frames;
    float msPerRender = 1000.f * renderTime / frames;
    float msPerRest = msPerSchedule - msPerRender;
    sxi::logger::info("Game closed successfully!");
    sxi::logger::info("---Stats---");
    sxi::logger::info("Frames rendered: " + std::to_string(frames));
    sxi::logger::info("Time active: " + std::to_string(timeActive));
    sxi::logger::info("Time spent rendering: " + std::to_string(renderTime));
    sxi::logger::info("---");
    sxi::logger::info("Avg. FPS: " + std::to_string((int)std::round(frames / timeActive)));
    sxi::logger::info("---");
    sxi::logger::info("Avg. ms/frame: " + std::to_string(1000.f * timeActive / frames) + "ms");
    sxi::logger::info("Avg. ms/schedule: " + std::to_string(msPerSchedule) + "ms");
    sxi::logger::info("Avg. ms/render: " + std::to_string(msPerRender) + "ms");
    sxi::logger::info("Avg. ms/rest: " + std::to_string(msPerRest) + "ms");
    sxi::logger::info("===Stats===");
}

static void loop()
{
    sxi::logger::trace("loop() started");

    SDL_Event e;
    SDL_zero(e);
    bool minimized = false;
    bool running = true;
    size_t frames = 0;
    sxi::TimePoint timeAtStart = timer.time;
    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                minimized = true;
            }

            if (e.type == SDL_EVENT_WINDOW_RESTORED)
            {
                minimized = false;
            }

            if (e.type == SDL_EVENT_QUIT)
            {
                printStats(timeAtStart, frames);
                return;
            }

            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                if (e.key.key == SDLK_ESCAPE)
                {
                    printStats(timeAtStart, frames);
                    return;
                }
            }
        }

        localTimer.refresh();
        scheduler->run();
        localTimer.refresh();
        schedulingTime += localTimer.dt;

        mgr.refresh();
        scheduler->refresh();
        timer.refresh();
        ++frames;
    }
}

int main(int argc, char* args[])
{
    std::ofstream f("/home/victordadaciu/workspace/StartingXI/log.txt");
    sxi::logger::init(f);
    sxi::logger::setName("MysteriousGame");
    sxi::logger::setLogLevel(sxi::logger::LogLevel::TRACE);

    scheduler = new sxi::JobScheduler();

    sxi::logger::trace("Initializing renderer");
    sxi::renderer::init(1600, 900);
    sxi::renderer::addGraphicsPipeline(sxi::file::readFileAsBytes(SHADERS_GEN_PATH + "basic_lighting.vert.spv"),
                                       sxi::file::readFileAsBytes(SHADERS_GEN_PATH + "basic_lighting.frag.spv"));

    sxi::ecs::ModelIndex mdl = sxi::renderer::addModel(MODELS_PATH + "human.obj");
    sxi::ecs::TextureIndex tex = sxi::renderer::addTexture(TEXTURES_PATH + "andrauv.png");

    sxi::logger::debug("Creating " + std::to_string(OBJ_SIDE * OBJ_SIDE) + " entities");
    for (int x = 0; x < OBJ_SIDE; ++x)
    {
        for (int z = 0; z < OBJ_SIDE; ++z)
        {
            sxi::ecs::EntityIndex<Object> ent = mgr.createEntity<Object>();
            sxi::ecs::RenderComponent& render = mgr.component<sxi::ecs::RenderComponent>(ent);
            render.mdl = mdl;
            render.tex = tex;
            sxi::ecs::PositionComponent& pos = mgr.component<sxi::ecs::PositionComponent>(ent);
            pos.pos = glm::vec3(x, 0, z);
        }
    }
    mgr.createEntity<sxi::ecs::Light>();
    mgr.refresh();

    sxi::logger::trace("Scheduling jobs");
    constexpr size_t chunkSize = 700;
    sxi::ScheduleId idPos = scheduler->schedule(
        [&](u32 start, u32 end)
        {
            mgr.forEntitiesMatching<MoveSignature>(
                [](auto&, auto& posComponent)
                {
                    static sxi::TimePoint start = timer.time;
                    posComponent.pos.y += 0.01f * sinf(2.f * sxi::Time::elapsed(start, timer.time));
                },
                start,
                end);
        },
        {SXI_CHECKPOINT_BEGIN},
        [&]()
        {
            return sxi::WorkSize(chunkSize, std::ceil(mgr.entityCount<Object>() / SXI_TO_FLOAT(chunkSize)));
        });

    sxi::ScheduleId idRot = scheduler->schedule(
        [&](u32 start, u32 end)
        {
            mgr.forEntitiesMatching<RotateSignature>(
                [](auto&, auto& rotComponent)
                {
                    rotComponent.rot.y += timer.dt;
                },
                start,
                end);
        },
        {SXI_CHECKPOINT_BEGIN},
        [&]()
        {
            return sxi::WorkSize(chunkSize, std::ceil(mgr.entityCount<Object>() / SXI_TO_FLOAT(chunkSize)));
        });

    sxi::ScheduleId idPos2 = scheduler->schedule(
        [](u32 start, u32 end)
        {
            mgr.forEntitiesMatching<MoveSignature>(
                [](auto&, auto& posComponent)
                {
                    posComponent.pos.y += 0.1f * timer.dt;
                },
                start,
                end);
        },
        {idPos},
        [&]()
        {
            return sxi::WorkSize(chunkSize, std::ceil(mgr.entityCount<Object>() / SXI_TO_FLOAT(chunkSize)));
        });

    scheduler->schedule(
        [&](u32 start, u32 end)
        {
            renderTimer.refresh();
            sxi::renderer::render(mgr, timer, start, end);
            renderTimer.refresh();
            renderTime += renderTimer.dt;
        },
        {idRot, idPos2});
    scheduler->refresh();

    loop();
    sxi::renderer::destroy();
    delete scheduler;

    return 0;
}