#pragma once
#include <concepts>


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
}
