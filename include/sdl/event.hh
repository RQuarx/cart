#pragma once
#include <concepts>

#include <SDL3/SDL_events.h>

#include "sdl/object.hh"


namespace cart::sdl
{
    using event_type = SDL_EventType;


    class event
    {
    public:
        constexpr event() noexcept : m_event {} {}
        constexpr event(const SDL_Event &event) noexcept : m_event { event } {}


        [[nodiscard]]
        constexpr auto type() const noexcept -> event_type
        { return event_type(m_event.type); }

        [[nodiscard]]
        constexpr auto get() const noexcept -> const SDL_Event &
        { return m_event; }


        [[nodiscard]]
        constexpr auto is_quit() const noexcept -> bool
        { return type() == event_type::SDL_EVENT_QUIT; }

        [[nodiscard]]
        constexpr auto is_window_event() const noexcept -> bool
        {
            return type() >= event_type::SDL_EVENT_WINDOW_FIRST
               and type() <= event_type::SDL_EVENT_WINDOW_LAST;
        }


        template <typename T, typename... EventTypes>
        [[nodiscard]]
        constexpr auto as(T SDL_Event::*event, EventTypes &&...event_types) const noexcept
            -> const T *
            requires(std::same_as<EventTypes, event_type> and ...)
        {
            if (((type() == event_types) or ...)) return &(m_event.*event);
            return nullptr;
        }


#define ACCESSOR(name, member_type, member_name, ...)                    \
        [[nodiscard]]                                                    \
        constexpr auto as_##name() const noexcept -> const member_type * \
        { return as(&SDL_Event::member_name, __VA_ARGS__); }

        ACCESSOR(key, SDL_KeyboardEvent, key, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP)
        ACCESSOR(text, SDL_TextInputEvent, text, SDL_EVENT_TEXT_INPUT)
        ACCESSOR(mouse_motion, SDL_MouseMotionEvent, motion, SDL_EVENT_MOUSE_MOTION)
        ACCESSOR(mouse_wheel, SDL_MouseWheelEvent, wheel, SDL_EVENT_MOUSE_WHEEL)
        ACCESSOR(mouse_button,
                 SDL_MouseButtonEvent,
                 button,
                 SDL_EVENT_MOUSE_BUTTON_DOWN,
                 SDL_EVENT_MOUSE_BUTTON_UP)
#undef ACCESSOR

        [[nodiscard]]
        constexpr auto as_window() const noexcept -> const SDL_WindowEvent *
        { return is_window_event() ? &m_event.window : nullptr; }

    private:
        SDL_Event m_event;
    };


    class event_pump final : object<>
    {
    public:
        /** @return The next pending event, or nullopt if the queue is empty. Never blocks. */
        [[nodiscard]] static auto poll() noexcept -> std::optional<event>;

        /** @brief Blocks until an event arrives. */
        [[nodiscard]] static auto wait() noexcept -> result<event>;

        /** @return nullopt if the timeout expired (or on error) without an event. */
        [[nodiscard]]
        static auto wait_for(std::chrono::milliseconds timeout) noexcept -> std::optional<event>;

        /** @brief Handle everything currently queued. */
        template <typename F>
        static void drain(F &&handler)
        {
            while (auto e = poll()) handler(*e);
        }

        /** @brief Post an event from any thread. */
        static auto push(const event &e) noexcept -> result<>;
        static auto push_quit() noexcept -> result<>;
    };
}
