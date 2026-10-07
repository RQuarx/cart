#pragma once
#include <concepts>
#include <memory>


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
    struct deleter_type
    {
        void operator()(P ptr) const noexcept
        {
            D(ptr);
            ptr = nullptr;
        }
    };


    template <typename T, auto D>
    class uptr
    {
    public:
        using value_type    = T;
        using pointer       = value_type *;
        using const_pointer = const value_type *;
        using deleter_type  = deleter_type<pointer, D>;


        constexpr uptr(pointer p = pointer()) noexcept : m_ptr { p } {}


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
    class sptr
    {
    public:
        using value_type    = T;
        using pointer       = value_type *;
        using const_pointer = const value_type *;
        using deleter_type  = deleter_type<pointer, D>;


        constexpr sptr(pointer p = pointer()) noexcept : m_ptr { p, deleter_type {} } {}


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
}
