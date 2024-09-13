#pragma once

#include "../structs.hpp"

#include <functional>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace nil::xit
{
    Binding<bool>& bind(
        Frame& frame,
        std::string tag,
        bool value,
        std::function<void(bool)> on_change = {}
    );

    Binding<double>& bind(
        Frame& frame,
        std::string tag,
        double value,
        std::function<void(double)> on_change = {}
    );

    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string tag,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change = {}
    );

    Binding<std::string>& bind(
        Frame& frame,
        std::string tag,
        std::string value,
        std::function<void(std::string_view)> on_change = {}
    );

    Binding<std::vector<std::uint8_t>>& bind(
        Frame& frame,
        std::string tag,
        std::vector<std::uint8_t> value,
        std::function<void(std::span<const std::uint8_t>)> on_change = {}
    );

    template <typename T>
        requires requires(T value) {
            { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
            { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
        }
    Binding<T>& bind(Frame& frame, std::string tag, T value)
    {
        auto& obj = bind(frame, std::move(tag), buffer_type<T>::serialize(value));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Binding<T>&>(obj);
    }

    template <typename T, typename U>
        requires requires(T value, U arg) {
            { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
            { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
        } && std::is_invocable_v<U, T>
    Binding<T>& bind(Frame& frame, std::string tag, T value, U on_change)
    {
        auto& obj = bind(
            frame,
            std::move(tag),
            buffer_type<T>::serialize(value),
            [on_change = std::move(on_change)](std::span<const std::uint8_t> v)
            {
                if (on_change)
                {
                    on_change(buffer_type<T>::deserialize(v.data(), v.size()));
                }
            }
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Binding<T>&>(obj);
    }
}
