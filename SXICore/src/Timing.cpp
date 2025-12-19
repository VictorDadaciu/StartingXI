#include "Timing.h"

namespace sxi
{
void Time::refresh()
{
    TimePoint now = Clock::now();
    dt = elapsed(time, now);
    time = now;
}
} // namespace sxi