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
              m_source { fmt.source }
        {
        }


        template <typename... Args>
        error(const std::source_location &source,
              std::format_string<Args...> fmt,
              Args &&...args) noexcept
            : std::runtime_error { std::format(fmt, std::forward<Args>(args)...) },
              m_source { source }
        {
        }


        [[nodiscard]] constexpr auto where() const noexcept -> std::source_location
        { return this->m_source; }


        template <typename T>
        [[nodiscard]]
        auto unexpected(this T &&self) noexcept -> std::unexpected<std::remove_cvref_t<T>>
        { return std::unexpected { std::forward<T>(self) }; }


        template <typename T>
        [[nodiscard]]
        auto format(this const T &self) -> std::string
        {
            if constexpr (std::is_same_v<T, error>)
                return std::format("[{}:{}:{}]: {}", self.m_source.file_name(),
                                   self.m_source.line(), self.m_source.column(), self.what());
            else
                return self.format();
        }

    private:
        std::source_location m_source;
    };


    template <typename T = void, std::derived_from<error> E = error>
    using result = std::expected<T, E>;

    enum class action : std::uint8_t
    {
        exit_success     = 0,
        exit_failure     = 1,
        continue_process = 2,
    };
}


template <>
struct std::formatter<cart::error>
{
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const cart::error &err, FormatContext &ctx) const -> FormatContext::iterator
    { return std::format_to(ctx.out(), "{}", err.format()); }
};
