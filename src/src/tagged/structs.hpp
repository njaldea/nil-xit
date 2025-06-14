#pragma once

#include <nil/xit/structs.hpp>
#include <nil/xit/tagged/structs.hpp>

#include <nil/service/ID.hpp>
#include <nil/xalt/transparent_stl.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace nil::xit
{
    struct Core;
}

namespace nil::xit::tagged
{
    template <typename T>
    struct Value
    {
        Frame* frame = nullptr;
        std::string id;
        std::unique_ptr<IAccessor<T>> accessor;
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
        std::optional<std::string> path;
        std::function<void(std::string_view)> on_load;
        std::function<void(std::string_view, std::size_t)> on_sub;
        nil::xalt::transparent_umap<std::vector<nil::service::ID>> subscribers;

        using Value_t = std::variant<
            Value<bool>,
            Value<std::int64_t>,
            Value<double>,
            Value<std::string>,
            Value<std::vector<std::uint8_t>>>;
        nil::xalt::transparent_umap<Value_t> values;

        using Signal_t = std::variant<
            Signal<void>,
            Signal<bool>,
            Signal<std::int64_t>,
            Signal<double>,
            Signal<std::string_view>,
            Signal<std::span<const std::uint8_t>>>;
        nil::xalt::transparent_umap<Signal_t> signals;
    };
}
