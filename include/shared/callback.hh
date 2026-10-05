#pragma once
#include <functional>


namespace cart
{
    template <typename Signature>
    class callback;

    template <typename R, typename... Args>
    class callback<auto(Args...)->R>
    {
    public:
        callback() = default;

        template <typename Self>
        callback(Self &self, auto (Self::*fn)(Args...)->R)
            : m_fn { [&self, fn](Args... args) -> R
                     { return (self.*fn)(std::forward<Args>(args)...); } }
        {
        }


        template <typename Self>
        callback(Self *self, auto (Self::*fn)(Args...)->R)
            : m_fn { [self, fn](Args... args) -> R
                     { return (self->*fn)(std::forward<Args>(args)...); } }
        {
        }


        template <typename Self>
        void set(Self &self, auto (Self::*fn)(Args...)->R)
        {
            m_fn = [&self, fn](Args... args) -> R
            { return (self.*fn)(std::forward<Args>(args)...); };
        }


        template <typename Self>
        void set(Self *self, auto (Self::*fn)(Args...)->R)
        {
            m_fn = [self, fn](Args... args) -> R
            { return (self->*fn)(std::forward<Args>(args)...); };
        }

        auto operator()(Args... args) -> R { return m_fn(std::forward<Args>(args)...); }

    private:
        std::function<auto(Args...)->R> m_fn;
    };
}
