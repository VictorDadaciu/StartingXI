#include "JobScheduler.h"

#include "Types.h"

#include <cassert>
#include <mutex>
#include <vector>

namespace sxi
{
namespace detail
{
    void Job::createAndRunTasks(ScheduleId id, detail::ThreadPool& threadPool)
    {
        WorkSize workSize = m_workSizeCalculator();
        assert(workSize.chunkSize > 0);
        assert(workSize.numChunks > 0);

        m_tasksCounter = workSize.numChunks;

        size_t start = 0;
        size_t end = workSize.chunkSize;
        for (size_t chunk = 0; chunk < workSize.numChunks; ++chunk)
        {
            threadPool.emplace(id,
                               m_work,
                               start,
                               end,
                               [this]()
                               {
                                   this->taskFinished();
                               });
            start = end;
            end += workSize.chunkSize;
        }
    }

    void Job::taskFinished()
    {
        if (--m_tasksCounter == 0)
        {
            m_jobFinished();
        }
    }
} // namespace detail

JobScheduler::JobScheduler(size_t numThreads) : m_threadPool(numThreads)
{
    assert(numThreads > 0);

    m_checkpoints.emplace_back(SXI_CHECKPOINT_BEGIN);
}

ScheduleId JobScheduler::schedule(const Work& work,
                                  std::vector<ScheduleId>&& prereqs,
                                  const WorkSizeCalculator& workSizeCalculator)
{
    ScheduleId id = setCheckpoint(std::move(prereqs));
    m_checkpoints[id].setJob(detail::JobIndex{SXI_TO_I32(m_jobs.size())});
    m_jobs.emplace_back(work,
                        workSizeCalculator,
                        [this, id]()
                        {
                            this->jobFinished(id);
                        });

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

        m_checkpoints[parentId].addChild(id);
    }

    return id;
}

void JobScheduler::refresh() noexcept
{
    for (detail::Checkpoint& checkpoint : m_checkpoints)
    {
        for (ScheduleId id : checkpoint.children())
        {
            m_checkpoints[id].incrementParentCounter();
        }
    }
}

void JobScheduler::jobFinished(ScheduleId id)
{
    detail::Checkpoint& checkpoint = m_checkpoints[id];
    for (ScheduleId childId : checkpoint.children())
    {
        detail::Checkpoint& child = m_checkpoints[childId];
        if (child.prerequisiteJobFinished() > 0)
        {
            continue;
        }

        // needs to trickle down
        if (child.jobEnabled())
        {
            m_jobs[child.jobIndex()].createAndRunTasks(childId, m_threadPool);
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

    m_checkpoints[id].setJobEnabled(enabled);
}

bool JobScheduler::isEnabled(ScheduleId id) const noexcept
{
    assert(id < m_checkpoints.size());

    return m_checkpoints[id].jobEnabled();
}
} // namespace sxi