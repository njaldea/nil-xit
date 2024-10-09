#pragma once

#include <nil/xit/structs.hpp>

#include <nil/service/structs.hpp>

#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>

namespace nil::xit
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
    struct TaggedBinding // NOLINT
    {
        TaggedFrame* frame;
        std::string id;
        std::function<T(std::string_view)> getter;
        std::function<void(std::string_view, const T&)> setter;
    };

    template <>
    struct TaggedBinding<std::string>
    {
        TaggedFrame* frame;
        std::string id;
        std::function<std::string(std::string_view)> getter;
        std::function<void(std::string_view, std::string_view)> setter;
    };

    template <>
    struct TaggedBinding<std::vector<std::uint8_t>>
    {
        TaggedFrame* frame;
        std::string id;
        std::function<std::vector<std::uint8_t>(std::string_view)> getter;
        std::function<void(std::string_view, std::span<const std::uint8_t>)> setter;
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

    template <typename T>
    struct TaggedListener
    {
        std::function<void(std::string_view, const T&)> on_change;
    };

    template <>
    struct TaggedListener<void>
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

    struct TaggedFrame
    {
        Core* core;
        std::string id;
        std::filesystem::path path;

        using TaggedBinding_t = std::variant<
            TaggedBinding<bool>,
            TaggedBinding<std::int64_t>,
            TaggedBinding<double>,
            TaggedBinding<std::string>,
            TaggedBinding<std::vector<std::uint8_t>>>;
        std::unordered_map<std::string, TaggedBinding_t> bindings;

        using TaggedListener_t = std::variant<
            TaggedListener<void>,
            TaggedListener<bool>,
            TaggedListener<std::int64_t>,
            TaggedListener<double>,
            TaggedListener<std::string_view>,
            TaggedListener<std::span<const std::uint8_t>>>;
        std::unordered_map<std::string, TaggedListener_t> listeners;
    };

    struct Core
    {
        nil::service::MessagingService& service; // NOLINT
        std::filesystem::path cache_location;
        std::unordered_map<std::string, std::variant<Frame, TaggedFrame>> frames;
    };
}
