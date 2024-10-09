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
    struct Binding // NOLINT
    {
        Frame* frame;
        std::string id;
        std::function<T(std::string_view)> getter;
        std::function<void(std::string_view, const T&)> setter;
    };

    template <>
    struct Binding<std::string>
    {
        Frame* frame;
        std::string id;
        std::function<std::string(std::string_view)> getter;
        std::function<void(std::string_view, std::string_view)> setter;
    };

    template <>
    struct Binding<std::vector<std::uint8_t>>
    {
        Frame* frame;
        std::string id;
        std::function<std::vector<std::uint8_t>(std::string_view)> getter;
        std::function<void(std::string_view, std::span<const std::uint8_t>)> setter;
    };

    template <typename T>
    struct Listener
    {
        std::function<void(std::string_view, const T&)> on_change;
    };

    template <>
    struct Listener<void>
    {
        std::function<void(std::string_view)> on_change;
    };

    struct Frame
    {
        Core* core;
        std::string id;
        std::filesystem::path path;

        using Binding_t = std::variant<
            Binding<bool>,
            Binding<std::int64_t>,
            Binding<double>,
            Binding<std::string>,
            Binding<std::vector<std::uint8_t>>>;
        std::unordered_map<std::string, Binding_t> bindings;

        using Listener_t = std::variant<
            Listener<void>,
            Listener<bool>,
            Listener<std::int64_t>,
            Listener<double>,
            Listener<std::string_view>,
            Listener<std::span<const std::uint8_t>>>;
        std::unordered_map<std::string, Listener_t> listeners;
    };
}
