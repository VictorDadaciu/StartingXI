#pragma once

#include <chrono>

namespace sxi
{
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::high_resolution_clock::time_point;
inline constexpr float SXI_DT_144FPS = 1.f / 144.f;

struct Time final
{
    Time() : time(Clock::now()) {}

    Time(float initialDT) : time(Clock::now()), dt(initialDT) {}

    template<class Duration = std::chrono::seconds>
    static float elapsed(const TimePoint& before, const TimePoint& after)
    {
        static_assert(std::chrono::__is_duration_v<Duration>, "");

        return std::chrono::duration<float, typename Duration::period>(after - before).count();
    }

    void refresh();

    TimePoint time{};
    float dt = SXI_DT_144FPS;
};
} // namespace sxi
