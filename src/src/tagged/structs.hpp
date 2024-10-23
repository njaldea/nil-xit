#pragma once

#include <nil/xit/tagged/structs.hpp>

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

namespace nil::xit::tagged
{
    template <typename T>
    struct Value // NOLINT
    {
        Frame* frame;
        std::string id;
        std::function<T(std::string_view)> getter;
        std::function<void(std::string_view, const T&)> setter;
    };

    template <>
    struct Value<std::string>
    {
        Frame* frame;
        std::string id;
        std::function<std::string(std::string_view)> getter;
        std::function<void(std::string_view, std::string_view)> setter;
    };

    template <>
    struct Value<std::vector<std::uint8_t>>
    {
        Frame* frame;
        std::string id;
        std::function<std::vector<std::uint8_t>(std::string_view)> getter;
        std::function<void(std::string_view, std::span<const std::uint8_t>)> setter;
    };

    template <typename T>
    struct Signal
    {
        std::function<void(std::string_view, const T&)> on_call;
    };

    template <>
    struct Signal<void>
    {
        std::function<void(std::string_view)> on_call;
    };

    struct Frame
    {
        Core* core;
        std::string id;
        std::filesystem::path path;

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
