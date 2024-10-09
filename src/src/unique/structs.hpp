#pragma once

#include <nil/xit/unique/structs.hpp>

#include "../structs.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <variant>

namespace nil::xit::unique
{
    template <typename T>
    struct Binding // NOLINT
    {
        Frame* frame;
        std::string id;
        T value;
        std::function<void(const T&)> on_change;
    };

    template <>
    struct Binding<std::string>
    {
        Frame* frame;
        std::string id;
        std::string value;
        std::function<void(std::string_view)> on_change;
    };

    template <>
    struct Binding<std::vector<std::uint8_t>>
    {
        Frame* frame;
        std::string id;
        std::vector<std::uint8_t> value;
        std::function<void(std::span<const std::uint8_t>)> on_change;
    };

    template <typename T>
    struct Listener
    {
        std::function<void(const T&)> on_change;
    };

    template <>
    struct Listener<void>
    {
        std::function<void()> on_change;
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
