#pragma once
#include <concepts>
#include <memory>
#include <mutex>

#include "shared/result.hh"


namespace cart::trait
{
    /**
     * @brief Constrains `E` to be an enum whose underlying type is exactly `T`.
     *
     * @tparam E The enum type to check.
     * @tparam T The expected underlying integral type of `E`.
     *
     * Example usage:
     * @code{cpp}
     * enum class color : std::uint8_t { red, green, blue };
     *
     * static_assert(trait::enumtype_of<color, std::uint8_t>);
     * static_assert(!trait::enumtype_of<color, std::uint32_t>); // wrong underlying type
     * static_assert(!trait::enumtype_of<int, int>);             // not an enum
     * @endcode
     */
    template <typename E, typename T>
    concept enumtype_of = std::is_integral_v<T> and std::is_enum_v<E>
                      and std::is_same_v<std::underlying_type_t<E>, T>;


    /**
     * @brief Describes a policy that can acquire and release some global runtime state.
     *
     * A type satisfies `runtime_policy` if it provides two `noexcept` static functions:
     * - `T::init()` acquires the state, and returns a `result<>` describing success or
     *   failure.
     * - `T::deinit()` releases the state. It must not fail.
     *
     * Example usage:
     * @code{cpp}
     * struct my_library_policy
     * {
     *     static auto init() noexcept -> result<>
     *     {
     *         // Initialize the library here; return an error on failure.
     *         return {};
     *     }
     *
     *     static void deinit() noexcept
     *     {
     *         // Shut the library down here.
     *     }
     * };
     *
     * static_assert(trait::runtime_policy<my_library_policy>);
     * @endcode
     */
    template <typename T>
    concept runtime_policy = requires {
        { T::init() } noexcept -> std::same_as<result<>>;
        { T::deinit() } noexcept;
    };


    namespace _impl
    {
        template <typename P, auto D>
            requires std::is_pointer_v<P> and std::is_invocable_v<decltype(D), P>
        struct deleter;

        template <runtime_policy Policy>
        class runtime_state;

        template <template <typename...> typename Wrapper, typename T, auto D>
        class base_handle_of;
    }


    /**
     * @brief A trait that turns the class deriving from it into a bitpack.
     *
     * @tparam T The underlying integral type used to store the bits. It must match the
     *         underlying type of the enum whose values are used as flags.
     *
     * Example usage:
     * @code{cpp}
     * class attribute final : public trait::bitpack<std::uint8_t>
     * {
     * public:
     *     enum flag : underlying_type
     *     {
     *         a = 1 << 0,
     *         b = 1 << 1,
     *         c = 1 << 2,
     *     };
     * };
     *
     * attribute attr;
     *
     * attr.set(attribute::a, true);
     * attr.set(attribute::c, true);
     *
     * assert(attr.has(attribute::a) == true);
     * assert(attr.has(attribute::b) == false);
     * assert(attr.has(attribute::c) == true);
     *
     * attr.set(attribute::a, false);
     * assert(attr.has(attribute::a) == false);
     * @endcode
     */
    template <std::integral T>
    class bitpack
    {
    public:
        using underlying_type = T;


        /**
         * @brief Checks whether a flag is `true`/`1` in the bitpack.
         *
         * @param flag The flag to check.
         * @return `true` if any bit of `flag` is set, otherwise `false`.
         *
         * @tparam E The type of the enum, constrained to have the same underlying type as
         *         @ref bitpack<T>::underlying_type .
         */
        template <enumtype_of<underlying_type> E>
        [[nodiscard]]
        constexpr auto has(E flag) const noexcept -> bool
        { return (m_data & static_cast<underlying_type>(flag)) != 0; }


        /**
         * @brief Sets the bit(s) representing a flag to either `true` or `false`.
         *
         * @param flag  The flag to modify.
         * @param state The new state of the bit(s): `true` sets them, `false` clears them.
         *
         * @tparam E The type of the enum, constrained to have the same underlying type as
         *         @ref bitpack<T>::underlying_type .
         */
        template <enumtype_of<underlying_type> E>
        constexpr void set(E flag, bool state) noexcept
        {
            const auto value = static_cast<underlying_type>(flag);
            if (state)
                m_data |= value;
            else
                m_data &= ~value;
        }


        /** @brief Checks if the internal data is empty (0). */
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_data == 0; }

    private:
        underlying_type m_data = 0;
    };


    /**
     * @brief A trait that turns the class deriving from it into an owning wrapper of a
     *        raw pointer, managed by an `std::unique_ptr`.
     *
     * The pointer is released by calling `D` on it, unless it is `nullptr`.
     * The resulting class is move-only.
     *
     * @tparam T The pointee type (the handle wraps a `T *`).
     * @tparam D The destructor/deleter of `T *`. It must be invocable with a `T *`.
     *
     * Example usage:
     * @code{cpp}
     * #include <cstdio>
     *
     * class file final : public trait::unique_handle_of<std::FILE, std::fclose>
     * {
     * public:
     *     file(const char *path, const char *mode)
     *         : unique_handle_of { std::fopen(path, mode) }
     *     {}
     *
     *     [[nodiscard]]
     *     auto is_open() const noexcept -> bool
     *     { return get() != nullptr; }
     * };
     *
     * {
     *     file f { "data.txt", "r" };
     *     if (f.is_open())
     *         std::fputs("hello", f.get()); // use the raw pointer when needed
     * } // std::fclose is called automatically here
     * @endcode
     */
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


    /**
     * @brief A trait that turns the class deriving from it into a shared-ownership
     *        wrapper of a raw pointer, managed by an `std::shared_ptr`.
     *
     * The pointer is released by calling `D` on it, unless it is `nullptr`, once the last
     * copy of the handle is destroyed. The resulting class is copyable.
     *
     * @tparam T The pointee type (the handle wraps a `T *`).
     * @tparam D The destructor/deleter of `T *`. It must be invocable with a `T *`.
     *
     * Example usage:
     * @code{cpp}
     * // A C-style API, for illustration.
     * struct texture_impl;
     * texture_impl *texture_create(const char *path);
     * void          texture_destroy(texture_impl *tex);
     *
     *
     * class texture final : public trait::shared_handle_of<texture_impl, texture_destroy>
     * {
     * public:
     *     explicit texture(const char *path)
     *         : shared_handle_of { texture_create(path) }
     *     {}
     * };
     *
     * texture a { "wall.png" };
     * texture b = a;  // `a` and `b` share the same underlying texture
     *
     * // texture_destroy is called once, when both `a` and `b` are gone.
     * @endcode
     */
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


    /**
     * @brief A trait that makes its derived classes automatically ensure that some global
     *        state is initialized when the first derived object is constructed, and
     *        deinitialized when the last derived object is destroyed.
     *
     * The state is reference-counted per `Policy`, and access to the counter is
     * thread-safe. If `Policy::init()` fails, the constructor throws the error held by
     * the returned `result<>`.
     *
     * Copying or moving a derived object counts as creating a new one, so it also
     * acquires the state. This keeps the reference count correct.
     *
     * @tparam Policy The policy used to acquire and release the state.
     *         It must satisfy @ref runtime_policy .
     *
     * Example usage:
     * @code{cpp}
     * struct window_system_policy
     * {
     *     static auto init() noexcept -> result<>
     *     {
     *         // Start the windowing library; return an error on failure.
     *         return {};
     *     }
     *
     *     static void deinit() noexcept
     *     {
     *         // Shut the windowing library down.
     *     }
     * };
     *
     * class window final : private trait::runtime_guard<window_system_policy>
     * {
     * public:
     *     window() = default; // the guard has already initialized the library
     * };
     *
     * int main()
     * {
     *     window a;   // init() is called here
     *     window b;   // the count goes up; init() is not called again
     * }               // deinit() is called once, after the last window is destroyed
     * @endcode
     */
    template <runtime_policy Policy>
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


    namespace _impl
    {
        template <typename P, auto D>
            requires std::is_pointer_v<P> and std::is_invocable_v<decltype(D), P>
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
}
