#include "detail/ThreadPool.h"

#include "detail/Task.h"

#include <assert.h>
#include <functional>

namespace sxi::detail
{
ThreadPool::ThreadPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        m_threads.emplace_back(
            [this]()
            {
                while (true)
                {
                    Task task{};
                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);
                        m_cv.wait(lock,
                                  [this]()
                                  {
                                      return !m_tasks.empty() || m_stop;
                                  });

                        if (m_stop && m_tasks.empty())
                        {
                            return;
                        }

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                    task.run();
                }
            });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }

    m_cv.notify_all();
    for (std::thread& thread : m_threads)
    {
        thread.join();
    }
}

void ThreadPool::emplace(size_t id, const Work& work, size_t start, size_t end, const std::function<void()>& callback)
{
    assert(start < end);
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_tasks.emplace(id, work, start, end, callback);
    }
    m_cv.notify_one();
}
} // namespace sxi::detail