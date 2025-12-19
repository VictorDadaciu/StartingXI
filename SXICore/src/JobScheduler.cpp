#include "JobScheduler.h"

#include "Types.h"
#include "logger/Logger.h"

#include <cassert>
#include <mutex>
#include <string>
#include <vector>

namespace sxi
{
using namespace types;

namespace detail
{
    void Job::createAndRunTasks()
    {
        WorkSize workSize = m_workSizeCalculator();
        m_tasksCounter = workSize.m_numChunks;
        u32 start = 0;
        u32 end = workSize.m_chunkSize;
        for (u16 chunk = 0; chunk < workSize.m_numChunks; ++chunk)
        {
            s_threadPool->enqueue_detach(
                [this](u32 start, u32 end)
                {
                    m_work(start, end);
                    taskFinished();
                },
                start,
                end);
            start = end;
            end += workSize.m_chunkSize;
        }
    }

    void Job::taskFinished()
    {
        if (--m_tasksCounter == 0)
        {
            s_scheduler->jobFinished(m_scheduleId);
        }
    }
} // namespace detail

JobScheduler::JobScheduler(u8 numThreads) : m_threadPool(numThreads)
{
    assert(numThreads > 0);
    sxi::logger::trace("ThreadPool initialized with " + std::to_string(numThreads) + " threads");

    m_checkpoints.emplace_back(SXI_CHECKPOINT_BEGIN);
    detail::Job::s_scheduler = this;
    detail::Job::s_threadPool = &m_threadPool;
}

ScheduleId
JobScheduler::schedule(Work&& work, std::vector<ScheduleId>&& prereqs, WorkSizeCalculator&& workSizeCalculator)
{
    ScheduleId id = setCheckpoint(std::move(prereqs));
    m_checkpoints[id].setJob(detail::JobIndex{SXI_TO_I16(m_jobs.size())});
    m_jobs.emplace_back(*this, id, std::move(work), std::move(workSizeCalculator));

    return id;
}

ScheduleId JobScheduler::setCheckpoint(std::vector<ScheduleId>&& prereqs)
{
    assert(prereqs.size() > 0);

    ScheduleId id = ScheduleId(m_checkpoints.size());
    m_checkpoints.emplace_back(id);
    for (ScheduleId parentId : prereqs)
    {
        assert(parentId < m_checkpoints.size());

        m_checkpoints[parentId].m_children.push_back(id);
    }

    return id;
}

void JobScheduler::refresh() noexcept
{
    for (detail::Checkpoint& checkpoint : m_checkpoints)
    {
        for (ScheduleId id : checkpoint.m_children)
        {
            m_checkpoints[id].incrementParentCounter();
        }
    }
}

void JobScheduler::jobFinished(ScheduleId id)
{
    detail::Checkpoint& checkpoint = m_checkpoints[id];
    for (ScheduleId childId : checkpoint.m_children)
    {
        detail::Checkpoint& child = m_checkpoints[childId];
        if (child.prerequisiteJobFinished() > 0)
        {
            continue;
        }

        // needs to trickle down
        if (child.jobEnabled())
        {
            m_jobs[child.m_jobIndex].createAndRunTasks();
        }
        else
        {
            jobFinished(childId);
        }
    }

    if (--m_jobsLeft == 0)
    {
        m_cv.notify_one();
    }
}

void JobScheduler::run()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_jobsLeft.store(m_checkpoints.size());
    jobFinished(SXI_CHECKPOINT_BEGIN);
    m_cv.wait(lock);
}

void JobScheduler::setEnabled(ScheduleId id, bool enabled) noexcept
{
    assert(id < m_checkpoints.size());

    m_checkpoints[id].m_enabled = enabled;
}

bool JobScheduler::isEnabled(ScheduleId id) const noexcept
{
    assert(id < m_checkpoints.size());

    return m_checkpoints[id].jobEnabled();
}
} // namespace sxi