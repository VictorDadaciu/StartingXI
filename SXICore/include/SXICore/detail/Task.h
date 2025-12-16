#pragma once

#include <cassert>
#include <functional>
#include <limits.h>

namespace sxi
{
#define SXI_TASK_EVERYTHING UINT_MAX

using Work = std::function<void(size_t, size_t)>;
} // namespace sxi

namespace sxi::detail
{
struct Task final
{
public:
    Task() = default;

    Task(size_t jobId, const Work& work, size_t start, size_t end, const std::function<void()>& callback) :
        m_jobId(jobId), m_work(work), m_start(start), m_end(end), m_callback(callback)
    {
    }

    void run() const
    {
        assert(m_end > 0);

        m_work(m_start, m_end);
        m_callback();
    }

private:
    size_t m_jobId;
    Work m_work{};
    size_t m_start{};
    size_t m_end{};
    std::function<void()> m_callback{};
};
} // namespace sxi::detail