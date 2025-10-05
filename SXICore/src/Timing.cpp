#include "Timing.h"

namespace sxi
{
	Time::Time() : time(Clock::now()) {}
	Time::Time(float initialDT) : time(Clock::now()), dt(initialDT) {}

	void Time::refresh()
	{
		TimePoint now = Clock::now();
		dt = elapsed(now, time);
		time = now;
	}

	float Time::elapsed(const TimePoint& from, const TimePoint& to)
	{
		return std::chrono::duration<float, std::chrono::seconds::period>(from - to).count();
	}
}