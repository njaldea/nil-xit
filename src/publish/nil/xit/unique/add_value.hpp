#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace nil::xit::unique
{
    Value<bool>& add_value(
        Frame& frame,
        std::string id,
        bool value,
        std::function<void(bool)> on_change = {}
    );

    Value<double>& add_value(
        Frame& frame,
        std::string id,
        double value,
        std::function<void(double)> on_change = {}
    );

    Value<std::int64_t>& add_value(
        Frame& frame,
        std::string id,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change = {}
    );

    Value<std::string>& add_value(
        Frame& frame,
        std::string id,
        std::string value,
        std::function<void(std::string_view)> on_change = {}
    );

    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        std::vector<std::uint8_t> value,
        std::function<void(std::span<const std::uint8_t>)> on_change = {}
    );

    template <typename T>
        requires requires(T value) {
            { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
            { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
        }
    Value<T>& add_value(Frame& frame, std::string tag, T value)
    {
        auto& obj = add_value(frame, std::move(tag), buffer_type<T>::serialize(value));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<T>&>(obj);
    }

    template <typename T, typename U>
        requires requires(T value, U arg) {
            { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
            { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
        } && std::is_invocable_v<U, T>
    Value<T>& add_value(Frame& frame, std::string tag, T value, U on_change)
    {
        auto& obj = add_value(
            frame,
            std::move(tag),
            buffer_type<T>::serialize(value),
            std::function<void(std::span<const std::uint8_t>)>(
                [on_change = std::move(on_change)](std::span<const std::uint8_t> v)
                {
                    if (on_change)
                    {
                        on_change(buffer_type<T>::deserialize(v.data(), v.size()));
                    }
                }
            )
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<T>&>(obj);
    }
}
