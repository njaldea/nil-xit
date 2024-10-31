#pragma once

#include <nil/xit/unique/structs.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <variant>

namespace nil::xit
{
    struct Core;
}

namespace nil::xit::unique
{
    template <typename T>
    struct Value // NOLINT
    {
        Frame* frame;
        std::string id;
        T value;
        std::function<void(const T&)> on_change;
    };

    template <>
    struct Value<std::string>
    {
        Frame* frame;
        std::string id;
        std::string value;
        std::function<void(std::string_view)> on_change;
    };

    template <>
    struct Value<std::vector<std::uint8_t>>
    {
        Frame* frame;
        std::string id;
        std::vector<std::uint8_t> value;
        std::function<void(std::span<const std::uint8_t>)> on_change;
    };

    template <typename T>
    struct Signal
    {
        std::function<void(const T&)> on_call;
    };

    template <>
    struct Signal<void>
    {
        std::function<void()> on_call;
    };

    struct Frame
    {
        Core* core;
        std::string id;
        std::filesystem::path path;
        std::function<void()> on_load;

        using Value_t = std::variant<
            Value<bool>,
            Value<std::int64_t>,
            Value<double>,
            Value<std::string>,
            Value<std::vector<std::uint8_t>>>;
        std::unordered_map<std::string, Value_t> values;

        using Signal_t = std::variant<
            Signal<void>,
            Signal<bool>,
            Signal<std::int64_t>,
            Signal<double>,
            Signal<std::string_view>,
            Signal<std::span<const std::uint8_t>>>;
        std::unordered_map<std::string, Signal_t> signals;
    };
}
