#pragma once
#include <concepts>
#include <memory>
#include <mutex>

#include "shared/result.hh"


namespace cart::trait
{
    namespace _impl
    {
        template <typename E, typename T>
        concept enumtype = std::is_integral_v<T> and std::is_enum_v<E>
                       and std::is_same_v<std::underlying_type_t<E>, T>;


        template <typename T>
        concept runtime_policy = requires {
            { T::init() } noexcept -> std::same_as<result<>>;
            { T::deinit() } noexcept;
        };


        template <typename P, auto D>
            requires std::is_invocable_v<decltype(D), P>
        struct deleter
        {
            void operator()(P ptr) const noexcept
            {
                if (ptr != nullptr) D(ptr);
            }
        };


        template <runtime_policy Policy>
        class runtime_state
        {
        public:
            [[nodiscard]]
            auto acquire() noexcept -> result<>
            {
                std::scoped_lock lock { m_mtx };
                if (m_count++ == 0)
                    if (auto res = Policy::init(); !res)
                    {
                        m_count--;
                        return res;
                    }
                return {};
            }


            void release() noexcept
            {
                std::scoped_lock lock { m_mtx };
                if (--m_count == 0) Policy::deinit();
            }

        private:
            std::mutex  m_mtx;
            std::size_t m_count = 0;
        };


        template <template <typename...> typename Wrapper, typename T, auto D>
        class base_handle_of
        {
        public:
            using value_type   = T;
            using pointer      = value_type *;
            using deleter_type = _impl::deleter<pointer, D>;


            base_handle_of(pointer p = pointer {}) noexcept { reset(p); }


            [[nodiscard]]
            auto get() const noexcept -> pointer
            { return m_ptr.get(); }

            [[nodiscard]]
            auto get() noexcept -> pointer
            { return m_ptr.get(); }

        protected:
            void reset(pointer p = pointer {}) noexcept
            {
                if constexpr (deleter_in_type)
                    m_ptr.reset(p);
                else
                    m_ptr.reset(p, deleter_type {});
            }

        private:
            static constexpr bool deleter_in_type = requires { typename Wrapper<T, deleter_type>; };
            static consteval auto select_wrapper() noexcept
            {
                if constexpr (deleter_in_type)
                    return std::type_identity<Wrapper<T, deleter_type>>();
                else
                    return std::type_identity<Wrapper<T>>();
            }


            decltype(select_wrapper())::type m_ptr;
        };
    }


    template <std::integral T>
    class attribute
    {
    public:
        template <_impl::enumtype<T> E>
        [[nodiscard]]
        constexpr auto has(E flag) const noexcept -> bool
        { return (m_data & static_cast<T>(flag)) != 0; }


        template <_impl::enumtype<T> E>
        constexpr void set(E flag, bool state) noexcept
        {
            const auto value = static_cast<T>(flag);
            if (state)
                m_data |= value;
            else
                m_data &= ~value;
        }

    private:
        T m_data = 0;
    };


    template <typename T, auto D>
    class unique_handle_of : public _impl::base_handle_of<std::unique_ptr, T, D>
    {
        using base = _impl::base_handle_of<std::unique_ptr, T, D>;

    public:
        using base::base;
        using typename base::deleter_type;
        using typename base::pointer;
        using typename base::value_type;
    };


    template <typename T, auto D>
    class shared_handle_of : public _impl::base_handle_of<std::shared_ptr, T, D>
    {
        using base = _impl::base_handle_of<std::shared_ptr, T, D>;

    public:
        using base::base;
        using typename base::deleter_type;
        using typename base::pointer;
        using typename base::value_type;
    };


    template <_impl::runtime_policy Policy>
    class runtime_guard
    {
    protected:
        runtime_guard()
        {
            if (auto res = m_state.acquire(); !res) throw res.error();
        }

        ~runtime_guard() noexcept { m_state.release(); }

        runtime_guard(const runtime_guard & /* unused */) : runtime_guard {} {}
        runtime_guard(runtime_guard && /* unused */) noexcept : runtime_guard {} {}
        auto operator=(const runtime_guard &) -> runtime_guard & = default;
        auto operator=(runtime_guard &&) -> runtime_guard &      = default;

    private:
        inline static _impl::runtime_state<Policy> m_state;
    };
}
