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
            state ? m_data |= value : m_data &= ~value;
        }

    private:
        T m_data = 0;
    };


    template <typename P, auto D>
    struct deleter
    {
        void operator()(P ptr) const noexcept
        {
            if (ptr != nullptr) D(ptr);
        }
    };


    template <typename T, auto D>
    class unique_handle_of
    {
    public:
        using value_type    = T;
        using pointer       = value_type *;
        using const_pointer = const value_type *;
        using deleter_type  = deleter<pointer, D>;


        constexpr unique_handle_of(pointer p = pointer()) noexcept : m_ptr { p } {}


        [[nodiscard]]
        constexpr auto get() const noexcept -> const_pointer
        { return m_ptr.get(); }

        [[nodiscard]]
        constexpr auto get() noexcept -> pointer
        { return m_ptr.get(); }


    protected:
        constexpr void reset(pointer p = pointer()) noexcept { m_ptr.reset(p); }

    private:
        std::unique_ptr<value_type, deleter_type> m_ptr;
    };


    template <typename T, auto D>
    class shared_handle_of
    {
    public:
        using value_type    = T;
        using pointer       = value_type *;
        using const_pointer = const value_type *;
        using deleter_type  = deleter<pointer, D>;


        constexpr shared_handle_of(pointer p = pointer()) noexcept : m_ptr { p, deleter_type {} } {}


        [[nodiscard]]
        constexpr auto get() const noexcept -> const_pointer
        { return m_ptr.get(); }

        [[nodiscard]]
        constexpr auto get() noexcept -> pointer
        { return m_ptr.get(); }


    protected:
        constexpr void reset(pointer p = pointer()) noexcept { m_ptr.reset(p, deleter_type {}); }

    private:
        std::shared_ptr<value_type> m_ptr;
    };


    template <typename T>
    concept runtime_policy = requires {
        { T::init() } -> std::same_as<result<>>;
        { T::deinit() };
    };


    template <runtime_policy Policy>
    class runtime_guard
    {
    protected:
        runtime_guard()
        {
            if (auto res = m_state.acquire(); !res) throw res.error();
        }

        ~runtime_guard() { m_state.release(); }

        runtime_guard(const runtime_guard & /* unused */) : runtime_guard() {}
        runtime_guard(runtime_guard && /* unused */) noexcept : runtime_guard() {}
        auto operator=(const runtime_guard &) -> runtime_guard & = default;
        auto operator=(runtime_guard &&) -> runtime_guard &      = default;

    private:
        inline static class
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
        } m_state;
    };
}
