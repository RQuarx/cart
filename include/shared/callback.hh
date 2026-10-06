#pragma once
#include <functional>
#include <type_traits>
#include <utility>

namespace cart
{
    template <typename Signature>
    class callback;

    template <typename R, typename... Args>
    class callback<R(Args...)>
    {
    public:
        callback() = default;

        template <typename Self, typename Class>
        callback(Self &self, R (Class::*fn)(Args...)) { set(self, fn); }

        template <typename Self, typename Class>
        callback(Self &self, R (Class::*fn)(Args...) const) { set(self, fn); }

        template <typename Self, typename Class>
        callback(Self *self, R (Class::*fn)(Args...)) { set(self, fn); }

        template <typename Self, typename Class>
        callback(Self *self, R (Class::*fn)(Args...) const) { set(self, fn); }

        template <typename Self, typename Class>
        void set(Self &self, R (Class::*fn)(Args...))
        {
            m_fn = [p = &self, fn](Args... args) -> R
            { return (p->*fn)(std::forward<Args>(args)...); };
        }

        template <typename Self, typename Class>
        void set(Self &self, R (Class::*fn)(Args...) const)
        {
            m_fn = [p = &self, fn](Args... args) -> R
            { return (p->*fn)(std::forward<Args>(args)...); };
        }

        template <typename Self, typename Class>
        void set(Self *self, R (Class::*fn)(Args...))
        {
            m_fn = [self, fn](Args... args) -> R
            { return (self->*fn)(std::forward<Args>(args)...); };
        }

        template <typename Self, typename Class>
        void set(Self *self, R (Class::*fn)(Args...) const)
        {
            m_fn = [self, fn](Args... args) -> R
            { return (self->*fn)(std::forward<Args>(args)...); };
        }

        void reset() { m_fn = nullptr; }
        explicit operator bool() const { return static_cast<bool>(m_fn); }

        auto operator()(Args... args) const -> R
        {
            if (m_fn) return m_fn(std::forward<Args>(args)...);
            if constexpr (std::is_void_v<R>) return;
            else return R {};
        }

    private:
        std::function<R(Args...)> m_fn;
    };
}