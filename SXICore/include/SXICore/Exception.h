#pragma once

#include <exception>
#include <stdexcept>
#include <string>

namespace sxi
{
    class InitializationException : public std::exception
    {
    public:
        explicit InitializationException(const char* message) : msg(message) {}
        explicit InitializationException(const std::string& message) : msg(message) {}

        virtual ~InitializationException() noexcept {}

        virtual const char* what() const noexcept { return msg.c_str(); }

    protected:
        std::string msg;
    };

    class ResourceCreationException : public std::exception
    {
    public:
        explicit ResourceCreationException(const char* message) : msg(message) {}
        explicit ResourceCreationException(const std::string& message) : msg(message) {}

        virtual ~ResourceCreationException() noexcept {}

        virtual const char* what() const noexcept { return msg.c_str(); }

    protected:
        std::string msg;
    };

    class InvalidArgumentException : public std::exception
    {
    public:
        explicit InvalidArgumentException(const char* message) : msg(message) {}
        explicit InvalidArgumentException(const std::string& message) : msg(message) {}

        virtual ~InvalidArgumentException() noexcept {}

        virtual const char* what() const noexcept { return msg.c_str(); }

    protected:
        std::string msg;
    };

    class MemoryAllocationException : public std::exception
    {
    public:
        explicit MemoryAllocationException(const char* message) : msg(message) {}
        explicit MemoryAllocationException(const std::string& message) : msg(message) {}

        virtual ~MemoryAllocationException() noexcept {}

        virtual const char* what() const noexcept { return msg.c_str(); }

    protected:
        std::string msg;
    };
}