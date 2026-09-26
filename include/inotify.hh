#pragma once
#include <cstring>
#include <expected>
#include <filesystem>
#include <thread>

#include <linux/limits.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "error.hh"


namespace cart
{
    class inotify_error final : public error
    {
    public:
        template <typename... Args>
        inotify_error(_impl::format_string<Args...> fmt, Args &&...args)
            : error { "inotify: {}", std::format(fmt.fmt, std::forward<Args>(args)...) }
        {
        }
    };


    template <std::invocable<std::expected<std::reference_wrapper<const ::inotify_event>, error>> F>
    auto create_fs_watcher(const std::filesystem::path &path, F &&fn) noexcept
        -> std::expected<std::jthread, error>
    {
        int fd = ::inotify_init();
        if (fd < 0)
            return inotify_error { "Failed to initialize: {}", std::strerror(errno) }.unexpected();

        int wd = ::inotify_add_watch(fd, path.c_str(), IN_MODIFY);
        if (wd < 0)
        {
            auto err = inotify_error { "Failed to add watcher for {}: {}", path.c_str(),
                                       std::strerror(errno) };
            ::close(fd);
            return std::move(err).unexpected();
        }

        return std::jthread {
            [fd = fd, fn = std::forward<F>(fn)] mutable
            {
                alignas(::inotify_event) std::array<std::byte, 4096> buffer;

                while (true)
                {
                    ::ssize_t size = ::read(fd, buffer.data(), buffer.size());

                    if (size < 0)
                    {
                        if (errno == EINTR) continue;

                        fn(inotify_error { "Failed to read fd ({}): {}", fd, std::strerror(errno) }
                               .unexpected());
                        break;
                    }

                    if (size == 0)
                    {
                        fn(inotify_error { "::read() returned EOF." }.unexpected());
                        break;
                    }

                    for (std::size_t offset = 0; offset < std::size_t(size);)
                    {
                        const auto &event
                            = *reinterpret_cast<const ::inotify_event *>(buffer.data() + offset);
                        fn(std::ref(event));
                        offset += sizeof(::inotify_event) + event.len;
                    }
                }

                ::close(fd);
            }
        };
    }
}
