#pragma once
#include <chrono>
#include <expected>
#include <format>
#include <source_location>


namespace cart
{
    namespace _impl
    {
        template <typename... Args>
        struct format_string_ext
        {
            using time_point = std::chrono::time_point<std::chrono::system_clock>;


            std::format_string<Args...> fmt;
            std::source_location        source;


            template <typename T>
            constexpr format_string_ext(const T                    &fmt,
                                        const std::source_location &source
                                        = std::source_location::current())
                requires std::constructible_from<std::format_string<Args...>, T>
                : fmt { fmt }, source { source }
            {
            }
        };


        template <typename... Args>
        using format_string = std::type_identity_t<format_string_ext<Args...>>;
    }


    class error : public std::runtime_error
    {
    public:
        template <typename... Args>
        error(_impl::format_string<Args...> fmt, Args &&...args)
            : std::runtime_error { std::format(fmt.fmt, std::forward<Args>(args)...) },
              source { fmt.source }, time { std::chrono::system_clock::now().time_since_epoch() }
        {
        }


        [[nodiscard]]
        constexpr auto where() const noexcept -> std::source_location
        { return this->source; }

        [[nodiscard]]
        constexpr auto when() const noexcept -> std::chrono::nanoseconds
        { return this->time; }

        [[nodiscard]]
        auto unexpected() noexcept -> std::unexpected<error>
        { return std::unexpected { *this }; }

    private:
        std::source_location     source;
        std::chrono::nanoseconds time;
    };
}
