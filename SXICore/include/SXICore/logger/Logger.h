#pragma once

#include "../Timing.h"

#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <string>
#include <thread>

namespace sxi::logger
{
enum class LogLevel : uint8_t
{
    DEBUG = 0,
    TRACE = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
};

namespace detail
{
    // have to do load balancing
    struct LogMessage final
    {
        LogMessage() = default;

        LogMessage(LogLevel logLevel_, const std::string& msg_, const TimePoint& timePoint_) :
            logLevel(logLevel_), msg(msg_), timePoint(timePoint_)
        {
        }

        LogLevel logLevel;
        std::string msg;
        TimePoint timePoint{};
    };

    class Logger final
    {
    public:
        Logger(std::ostream&);

        void setLogLevel(LogLevel logLevel) noexcept { m_logLevel = logLevel; }

        LogLevel logLevel() const noexcept { return m_logLevel; }

        void setName(const std::string& name) noexcept { m_name = name; }

        const std::string& name() const noexcept { return m_name; }

        void log(LogLevel, const std::string&);

        ~Logger();

    private:
        std::string constructLogMessage(LogMessage&&) const;

        std::ostream& m_output = std::cout;
        std::string m_name{};
        LogLevel m_logLevel = LogLevel::INFO;
        std::queue<LogMessage> m_queue{};
        std::thread m_thread;
        std::mutex m_mutex{};
        std::condition_variable m_cv{};
        sxi::TimePoint m_beginning{};

        bool m_stop{};
    };

    extern std::unique_ptr<Logger> logger;
} // namespace detail

inline void init(std::ostream& ostream = std::cout)
{
    assert(!detail::logger);
    detail::logger = std::make_unique<detail::Logger>(ostream);
}

inline void setLogLevel(LogLevel logLevel)
{
    assert(detail::logger);
    detail::logger->setLogLevel(logLevel);
}

inline LogLevel logLevel()
{
    assert(detail::logger);
    return detail::logger->logLevel();
}

inline void setName(const std::string& name)
{
    assert(detail::logger);
    detail::logger->setName(name);
}

inline const std::string& name()
{
    assert(detail::logger);
    return detail::logger->name();
}

inline void debug(const std::string& msg)
{
    assert(detail::logger);
    detail::logger->log(LogLevel::DEBUG, msg);
}

inline void trace(const std::string& msg)
{
    assert(detail::logger);
    detail::logger->log(LogLevel::TRACE, msg);
}

inline void info(const std::string& msg)
{
    assert(detail::logger);
    detail::logger->log(LogLevel::INFO, msg);
}

inline void warn(const std::string& msg)
{
    assert(detail::logger);
    detail::logger->log(LogLevel::WARNING, msg);
}

inline void error(const std::string& msg)
{
    assert(detail::logger);
    detail::logger->log(LogLevel::ERROR, msg);
}
} // namespace sxi::logger