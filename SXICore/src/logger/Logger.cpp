#include "logger/Logger.h"

#include "Timing.h"

#include <cassert>
#include <ctime>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

namespace sxi::logger::detail
{
std::unique_ptr<Logger> logger{nullptr};

static std::string logLevelAsString(LogLevel logLevel)
{
    switch (logLevel)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::TRACE:
        return "TRACE";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    }
}

std::string Logger::constructLogMessage(LogMessage&& msg) const
{
    std::stringstream ss;
    ss << "[" << std::fixed << std::setprecision(6) << Time::elapsed(m_beginning, msg.timePoint) << "] "
       << logger->name() << " " << logLevelAsString(msg.logLevel) << ": " << msg.msg << "\n";
    return ss.str();
}

Logger::Logger(std::ostream& ostream) : m_output(ostream), m_logLevel(LogLevel::INFO)
{
    m_beginning = Clock::now();
    m_thread = std::thread(
        [this]()
        {
            while (true)
            {
                LogMessage msg;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);

                    m_cv.wait(lock,
                              [this]()
                              {
                                  return !m_queue.empty() || m_stop;
                              });

                    if (m_stop && m_queue.empty())
                    {
                        return;
                    }

                    msg = std::move(m_queue.front());
                    m_queue.pop();
                }
                m_output << constructLogMessage(std::move(msg));
            }
        });
}

void Logger::log(LogLevel logLevel, const std::string& msg)
{
    if (logLevel < m_logLevel)
    {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(logLevel, msg, Clock::now());
    }
    m_cv.notify_one();
}

Logger::~Logger()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_all();
    m_thread.join();
}
} // namespace sxi::logger::detail