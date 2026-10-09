#pragma once
#include <toml++/toml.hpp>

#include "shared/result.hh"


namespace cart::app::cfg::utils
{
    template <typename T>
    auto as(std::string_view key, const toml::node &node) -> T
        requires std::same_as<T, std::int64_t> or std::same_as<T, double> or std::same_as<T, bool>
    {
        if (const toml::value<T> *res = node.as<toml::value<T>>(); res != nullptr)
            return res->get();
        throw error { "Node \"{}\" doesn't have the correct type.", key };
    }


    template <typename T>
    auto as(std::string_view key, const toml::node &node) -> T
        requires std::is_integral_v<T> and std::is_arithmetic_v<T>
             and (!std::same_as<T, std::int64_t> and !std::same_as<T, bool>)
    {
        auto i = as<std::int64_t>(key, node);

        if constexpr (std::is_unsigned_v<T>)
        {
            if (i < 0)
                throw error { "Node \"{}\": {} is an invalid value, values must be >= 0.", key, i };
        }

        if (std::abs(i) > std::numeric_limits<T>::max())
            throw error { "Node \"{}\": {} is an our-of-range value.", key, i };

        return static_cast<T>(i);
    }


    template <typename T>
    auto as(std::string_view key, const toml::node &node) -> T
        requires std::is_floating_point_v<T> and (!std::same_as<T, double>)
    {
        auto d = as<double>(key, node);

        if (std::abs(d) > std::numeric_limits<float>::max())
            throw error { "Node \"{}\": {} is an our-of-range value.", key, d };
        return static_cast<T>(d);
    }


    template <typename T>
    auto as(std::string_view key, const toml::node &node) -> const T &
        requires std::same_as<T, std::string>
    {
        if (const toml::value<T> *res = node.as<toml::value<T>>(); res != nullptr)
            return res->get();
        throw error { "Node \"{}\" doesn't have the correct type.", key };
    }


    template <typename T>
    auto as(std::string_view key, const toml::node &node) -> const T &
        requires std::same_as<T, toml::table> or std::same_as<T, toml::array>
    {
        if (const T *res = node.as<T>(); res != nullptr) return *res;
        throw error { "Node \"{}\" doesn't have the correct type.", key };
    }


    template <typename T>
    auto get(const toml::table &table, std::string_view key) -> std::optional<T>
        requires std::is_trivially_copy_constructible_v<T>
    {
        const toml::node *value = table.get(key);
        if (value == nullptr) return std::nullopt;
        return as<T>(key, *value);
    }


    template <typename T>
    auto get(const toml::table &table, std::string_view key)
        -> std::optional<const toml::value<T> *>
    {
        const toml::node *value = table.get(key);
        if (value == nullptr) return std::nullopt;
        return &as<T>(key, *value);
    }


    template <typename T>
    auto get(const toml::table &table, std::string_view key)
        -> std::optional<const T *>
        requires std::same_as<T, toml::table> or std::same_as<T, toml::array>
    {
        const toml::node *value = table.get(key);
        if (value == nullptr) return std::nullopt;
        return &as<T>(key, *value);
    }
}
