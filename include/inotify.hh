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
    template <typename F>
    auto create_fs_watcher(const std::filesystem::path &path, F &&fn) noexcept
        -> std::expected<std::jthread, error>
    {
        int fd = ::inotify_init();
        if (fd < 0)
            return error { "Failed to initialize inotify: {}", std::strerror(errno) }.unexpected();

        int wd = ::inotify_add_watch(fd, path.c_str(), IN_MODIFY);
        if (wd < 0)
        {
            auto err
                = error { "Failed to add watcher for {}: {}", path.c_str(), std::strerror(errno) };
            ::close(fd);
            return err.unexpected();
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

                        fn(error { "Failed to read inotify's fd: {}", std::strerror(errno) }
                               .unexpected());
                        break;
                    }

                    if (size == 0)
                    {
                        fn(std::nullopt);
                        break;
                    }

                    for (std::size_t offset = 0; offset < std::size_t(size);)
                    {
                        const auto *event
                            = reinterpret_cast<const ::inotify_event *>(buffer.data() + offset);
                        fn(*event);
                        offset += sizeof(::inotify_event) + event->len;
                    }
                }

                ::close(fd);
            }
        };
    }
}
