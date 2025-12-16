#pragma once

#include "Task.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stddef.h>
#include <thread>
#include <vector>

namespace sxi::detail
{
class ThreadPool final
{
public:
    ThreadPool(size_t = std::thread::hardware_concurrency());

    ~ThreadPool();

    void emplace(size_t, const Work&, size_t, size_t, const std::function<void()>&);

private:
    std::vector<std::thread> m_threads{};
    std::queue<Task> m_tasks;
    std::mutex m_queueMutex{};
    std::condition_variable m_cv{};
    bool m_stop;
};
} // namespace sxi::detail