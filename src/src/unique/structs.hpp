#pragma once

#include <nil/xit/structs.hpp>
#include <nil/xit/unique/structs.hpp>

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

namespace nil::xit::unique
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
        std::optional<std::string> path;
        std::function<void()> on_load;
        std::function<void(std::size_t)> on_sub;
        std::vector<nil::service::ID> subscribers;

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
