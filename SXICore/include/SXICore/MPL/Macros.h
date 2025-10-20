# pragma once

#define SXI_MPL_TYPE(x) typename decltype(x)::type
#define SXI_MPL_FWD(x) ::std::forward<decltype(x)>(x)

#define SXI_MPL_STRONG_TYPEDEF(underlying, name)                             \
    class name final                                                         \
    {                                                                        \
    private:                                                                 \
        underlying value;                                                    \
                                                                             \
    public:                                                                  \
        inline name() = default;                                             \
        inline name(const name& x) = default;                                \
        inline name(name&& x) = default;                                     \
        inline name& operator=(const name& rhs) = default;                   \
        inline name& operator=(name&& rhs) = default;                        \
        inline constexpr explicit name(underlying x) noexcept : value{x} {}  \
        inline constexpr name& operator=(underlying rhs) noexcept            \
        {                                                                    \
            value = rhs;                                                     \
            return *this;                                                    \
        }                                                                    \
        inline constexpr operator const underlying&() const noexcept         \
        {                                                                    \
            return value;                                                    \
        }                                                                    \
        inline constexpr operator underlying&() noexcept { return value; }   \
        inline constexpr decltype(auto) operator==(const name& rhs) noexcept \
        {                                                                    \
            return value == rhs.value;                                       \
        }                                                                    \
        inline constexpr decltype(auto) operator!=(const name& rhs) noexcept \
        {                                                                    \
            return value != rhs.value;                                       \
        }                                                                    \
        inline constexpr decltype(auto) operator<(const name& rhs) noexcept  \
        {                                                                    \
            return value < rhs.value;                                        \
        }                                                                    \
        inline constexpr decltype(auto) operator>(const name& rhs) noexcept  \
        {                                                                    \
            return value > rhs.value;                                        \
        }                                                                    \
        inline constexpr decltype(auto) operator<=(const name& rhs) noexcept \
        {                                                                    \
            return value <= rhs.value;                                       \
        }                                                                    \
        inline constexpr decltype(auto) operator>=(const name& rhs) noexcept \
        {                                                                    \
            return value >= rhs.value;                                       \
        }                                                                    \
    };