#pragma once
#include <memory>


namespace cart::sdl
{
    namespace _impl
    {
        template <typename P, auto D>
        struct deleter_type
        {
            void operator()(P ptr) const noexcept
            {
                D(ptr);
                ptr = nullptr;
            }
        };
    }


    template <typename T, auto D>
    class uptr
    {
    public:
        using value_type    = T;
        using pointer       = value_type *;
        using const_pointer = const value_type *;
        using deleter_type  = _impl::deleter_type<pointer, D>;


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
        using deleter_type  = _impl::deleter_type<pointer, D>;


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
