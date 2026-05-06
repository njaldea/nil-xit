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

namespace nil::xit
{
    struct Core;
}

namespace nil::xit::unique
{
    template <typename T>
    struct Value
    {
        using type = T;
        Frame* frame = nullptr;
        std::string id;
        std::unique_ptr<IAccessor<T>> accessor;
    };

    template <typename T>
    struct Signal
    {
        std::function<void(const T&)> on_call;
    };

    struct Frame
    {
        Core* core;
        std::string id;
        std::optional<FileInfo> file_info;
        std::function<void()> on_load;
        std::function<void(std::size_t)> on_sub;
        std::vector<nil::service::ID> subscribers;
        nil::xalt::transparent_umap<Value<std::vector<std::uint8_t>>> values;
        nil::xalt::transparent_umap<Signal<std::span<const std::uint8_t>>> signals;
    };
}
