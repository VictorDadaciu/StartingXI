#pragma once

#include <chrono>

namespace sxi
{
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::high_resolution_clock::time_point;
	inline constexpr float SXI_DT_144FPS = 1.f / 144.f;

	struct Time
	{
		Time();
		Time(float);

		static float elapsed(const TimePoint&, const TimePoint&);

		void refresh();

		TimePoint time{};
		float dt = SXI_DT_144FPS;
	};
}

