#include "ECSSettings.h"
#include "SXICore/ECS/Manager.h"
#include "SXICore/File.h"
#include "SXICore/JobScheduler.h"
#include "SXICore/Timing.h"
#include "SXICore/Types.h"
#include "SXICore/logger/Logger.h"
#include "SXIRenderer/ECSConfig.h"
#include "SXIRenderer/Renderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <limits>
#include <string>

const std::string BASE_PATH = "/home/victordadaciu/workspace/StartingXI/MysteriousGame/";
const std::string MODELS_PATH = BASE_PATH + "models/";
const std::string TEXTURES_PATH = BASE_PATH + "textures/";
const std::string SHADERS_PATH = BASE_PATH + "shaders/";
const std::string SHADERS_GEN_PATH = SHADERS_PATH + "generated/";

constexpr uint32_t frameThreshold = 200;

using namespace sxi::types;
constexpr u32 OBJ_SIDE = 70;

static sxi::ecs::Manager<ECSSettings> mgr;
static sxi::JobScheduler* scheduler{};
static size_t frames = 0;
static sxi::Time timer{};
static sxi::Time jobsTimer{};
static sxi::Time sdlTimer{};
static float minFrameTime = std::numeric_limits<float>::max();
static float maxFrameTime = std::numeric_limits<float>::min();
static float minRenderTime = std::numeric_limits<float>::max();
static float maxRenderTime = std::numeric_limits<float>::min();
static float totalRenderTime{};
static float minJobsTime = std::numeric_limits<float>::max();
static float maxJobsTime = std::numeric_limits<float>::min();
static float totalJobsTime{};
static float minSDLTime = std::numeric_limits<float>::max();
static float maxSDLTime = std::numeric_limits<float>::min();
static float totalSDLTime{};

void printStats(const sxi::TimePoint& timeAtStart, size_t frames)
{
    float timeActive = sxi::Time::elapsed(timeAtStart, timer.time);
    sxi::logger::info("Game closed successfully!");
    sxi::logger::info("---Stats---");
    sxi::logger::info("Frames rendered:  " + std::to_string(frames));
    sxi::logger::info("Time active:      " + std::to_string(timeActive));
    sxi::logger::info("---");
    sxi::logger::info("Avg. FPS:         " + std::to_string((int)std::round(frames / timeActive)));
    sxi::logger::info("Avg. frame time:  " + std::to_string(1000.f * timeActive / frames) + "ms");
    sxi::logger::info("Min frame time:   " + std::to_string(1000.f * minFrameTime) + "ms");
    sxi::logger::info("Max frame time:   " + std::to_string(1000.f * maxFrameTime) + "ms");
    sxi::logger::info("---");
    sxi::logger::info("Avg. render time: " + std::to_string(1000.f * totalRenderTime / frames) + "ms");
    sxi::logger::info("Min render time:  " + std::to_string(1000.f * minRenderTime) + "ms");
    sxi::logger::info("Max render time:  " + std::to_string(1000.f * maxRenderTime) + "ms");
    sxi::logger::info("---");
    sxi::logger::info("Avg. jobs time:   " + std::to_string(1000.f * totalJobsTime / frames) + "ms");
    sxi::logger::info("Min jobs time:    " + std::to_string(1000.f * minJobsTime) + "ms");
    sxi::logger::info("Max jobs time:    " + std::to_string(1000.f * maxJobsTime) + "ms");
    sxi::logger::info("---");
    sxi::logger::info("Avg. SDL time:    " + std::to_string(1000.f * totalSDLTime / frames) + "ms");
    sxi::logger::info("Min SDL time:     " + std::to_string(1000.f * minSDLTime) + "ms");
    sxi::logger::info("Max SDL time:     " + std::to_string(1000.f * maxSDLTime) + "ms");
    sxi::logger::info("===Stats===");
}

static void loop()
{
    SDL_Event e;
    SDL_zero(e);
    bool minimized = false;
    bool running = true;
    sxi::TimePoint timeAtStart = timer.time;
    while (running)
    {
        sdlTimer.refresh();
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
        sdlTimer.refresh();

        if (frames > frameThreshold)
        {
            minSDLTime = std::min(minSDLTime, sdlTimer.dt);
            maxSDLTime = std::max(maxSDLTime, sdlTimer.dt);
            totalSDLTime += sdlTimer.dt;
        }

        jobsTimer.refresh();
        scheduler->run();

        mgr.refresh();
        scheduler->refresh();
        timer.refresh();
        if (frames > frameThreshold)
        {
            minFrameTime = std::min(minFrameTime, timer.dt);
            maxFrameTime = std::max(maxFrameTime, timer.dt);
        }
        ++frames;
    }
}

int main(int argc, char* args[])
{
    // std::ofstream f("/home/victordadaciu/workspace/StartingXI/log.txt");
    sxi::logger::init();
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
    constexpr size_t numChunks = 4;
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
            return sxi::WorkSize(std::ceil(mgr.entityCount<Object>() / SXI_TO_FLOAT(numChunks)), numChunks);
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
            return sxi::WorkSize(std::ceil(mgr.entityCount<Object>() / SXI_TO_FLOAT(numChunks)), numChunks);
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
            return sxi::WorkSize(std::ceil(mgr.entityCount<Object>() / SXI_TO_FLOAT(numChunks)), numChunks);
        });

    scheduler->schedule(
        [&](u32 start, u32 end)
        {
            jobsTimer.refresh();
            if (frames > frameThreshold)
            {
                minJobsTime = std::min(minJobsTime, jobsTimer.dt);
                maxJobsTime = std::max(maxJobsTime, jobsTimer.dt);
                totalJobsTime += jobsTimer.dt;
            }
            sxi::renderer::render(mgr, timer, start, end);
            jobsTimer.refresh();
            if (frames > frameThreshold)
            {
                minRenderTime = std::min(minRenderTime, jobsTimer.dt);
                maxRenderTime = std::max(maxRenderTime, jobsTimer.dt);
                totalRenderTime += jobsTimer.dt;
            }
        },
        {idRot, idPos2});
    scheduler->refresh();

    loop();
    sxi::renderer::destroy();
    delete scheduler;

    return 0;
}