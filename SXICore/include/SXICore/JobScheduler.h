#pragma once

#include "MPL/Macros.h"
#include "Types.h"
#include "detail/ThreadPool.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace sxi
{
SXI_MPL_STRONG_TYPEDEF(size_t, ScheduleId)

struct WorkSize final
{
    size_t chunkSize = SXI_TASK_EVERYTHING;
    size_t numChunks = 1;
};

using WorkSizeCalculator = std::function<WorkSize()>;

namespace detail
{
    SXI_MPL_STRONG_TYPEDEF(int32_t, JobIndex)

    class Checkpoint final
    {
    public:
        Checkpoint(ScheduleId id) : m_id(id), m_jobIndex(JobIndex{-1}), m_enabled(false) {}

        Checkpoint(const Checkpoint& other) :
            m_id(other.m_id), m_jobIndex(other.m_jobIndex), m_enabled(other.m_enabled), m_children(other.m_children),
            m_parentCounter(other.m_parentCounter.load())
        {
        }

        [[nodiscard]] u8 prerequisiteJobFinished() noexcept { return --m_parentCounter; };

        void incrementParentCounter() { ++m_parentCounter; };

        const std::vector<ScheduleId>& children() const noexcept { return m_children; }

        void addChild(ScheduleId id) { m_children.push_back(id); }

        void setJob(JobIndex jobIndex) noexcept
        {
            m_jobIndex = jobIndex;
            m_enabled = true;
        }

        bool jobEnabled() const noexcept { return m_enabled && m_jobIndex > -1; }

        void setJobEnabled(bool enabled) noexcept { m_enabled = enabled; }

        JobIndex jobIndex() const noexcept { return m_jobIndex; }

    private:
        ScheduleId m_id;
        JobIndex m_jobIndex;
        bool m_enabled;
        std::vector<ScheduleId> m_children{};
        std::atomic_uint8_t m_parentCounter{};
    };

    class Job final
    {
    public:
        Job(const Work& work, const WorkSizeCalculator& workSizeCalculator, const std::function<void()>& jobFinished) :
            m_work(work), m_workSizeCalculator(workSizeCalculator), m_jobFinished(jobFinished)
        {
        }

        Job(const Job& other) :
            m_work(other.m_work), m_workSizeCalculator(other.m_workSizeCalculator), m_jobFinished(other.m_jobFinished),
            m_tasksCounter(other.m_tasksCounter.load())
        {
        }

        void createAndRunTasks(ScheduleId, detail::ThreadPool&);

        void taskFinished();

    private:
        Work m_work;
        WorkSizeCalculator m_workSizeCalculator;
        std::function<void()> m_jobFinished;
        std::atomic_uint8_t m_tasksCounter{};
    };
} // namespace detail

#define SXI_CHECKPOINT_BEGIN sxi::ScheduleId{0}

class JobScheduler final
{
public:
    // from main thread at initialization
    JobScheduler(size_t = std::thread::hardware_concurrency());

    // from main thread at initialization
    ScheduleId schedule(
        const Work&,
        std::vector<ScheduleId>&&,
        const WorkSizeCalculator& =
            []()
        {
            return WorkSize();
        });

    // from main thread at initialization
    ScheduleId setCheckpoint(std::vector<ScheduleId>&&);

    // from main thread at initialization
    void refresh() noexcept;

    void run();

    void setEnabled(ScheduleId, bool) noexcept;

    bool isEnabled(ScheduleId id) const noexcept;

private:
    void jobFinished(ScheduleId);

    detail::ThreadPool m_threadPool{};
    std::vector<detail::Checkpoint> m_checkpoints{};
    std::vector<detail::Job> m_jobs{};
    std::atomic_uint32_t m_jobsLeft{};
    std::condition_variable m_cv{};
    std::mutex m_mutex;
};
} // namespace sxi