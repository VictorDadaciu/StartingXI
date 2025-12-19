#pragma once

#include "MPL/Macros.h"
#include "Types.h"
#include "detail/Task.h"

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread_pool/thread_pool.h>
#include <vector>

namespace sxi
{
SXI_MPL_STRONG_TYPEDEF(uint16_t, ScheduleId)

namespace detail
{
    struct Job;
}

struct WorkSize final
{
    WorkSize() : m_chunkSize(SXI_TASK_EVERYTHING), m_numChunks(1) {}

    WorkSize(u16 chunkSize, u16 numChunks) : m_chunkSize(chunkSize), m_numChunks(numChunks)
    {
        assert(m_chunkSize > 0);
        assert(m_numChunks > 0);
    }

private:
    u16 m_chunkSize;
    u16 m_numChunks;

    friend class sxi::detail::Job;
};

using WorkSizeCalculator = std::function<WorkSize()>;

class JobScheduler;

namespace detail
{
    using namespace sxi::types;
    SXI_MPL_STRONG_TYPEDEF(int16_t, JobIndex)

    struct Checkpoint final
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

        void setJob(JobIndex jobIndex) noexcept
        {
            m_jobIndex = jobIndex;
            m_enabled = true;
        }

        bool jobEnabled() const noexcept { return m_enabled && m_jobIndex > -1; }

        ScheduleId m_id;
        JobIndex m_jobIndex;
        bool m_enabled;
        std::vector<ScheduleId> m_children{};

    private:
        std::atomic_uint8_t m_parentCounter{};
    };

    struct Job final
    {
    public:
        Job(JobScheduler& scheduler, ScheduleId scheduleId, Work&& work, WorkSizeCalculator&& workSizeCalculator) :
            m_scheduleId(scheduleId), m_work(work), m_workSizeCalculator(workSizeCalculator)
        {
            assert(s_scheduler != nullptr);
        }

        Job(const Job& other) :
            m_scheduleId(other.m_scheduleId), m_work(other.m_work), m_workSizeCalculator(other.m_workSizeCalculator),
            m_tasksCounter(other.m_tasksCounter.load())
        {
            assert(s_scheduler != nullptr);
        }

        void createAndRunTasks();

        void taskFinished();

        ScheduleId m_scheduleId;
        Work m_work;
        WorkSizeCalculator m_workSizeCalculator;
        inline static JobScheduler* s_scheduler{};
        inline static dp::thread_pool<>* s_threadPool{};

    private:
        std::atomic_uint8_t m_tasksCounter{};
    };
} // namespace detail

#define SXI_CHECKPOINT_BEGIN sxi::ScheduleId{0}

class JobScheduler final
{
public:
    // from main thread at initialization
    JobScheduler(u8 = std::thread::hardware_concurrency());

    // from main thread at initialization
    ScheduleId schedule(
        Work&&,
        std::vector<ScheduleId>&&,
        WorkSizeCalculator&& =
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

    dp::thread_pool<> m_threadPool;
    std::vector<detail::Checkpoint> m_checkpoints{};
    std::vector<detail::Job> m_jobs{};
    std::atomic_uint16_t m_jobsLeft{};
    std::condition_variable m_cv{};
    std::mutex m_mutex;

    friend struct detail::Job;
};
} // namespace sxi