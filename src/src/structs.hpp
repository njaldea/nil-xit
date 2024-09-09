#pragma once

#include <nil/xit/structs.hpp>

#include <nil/service/IService.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

namespace nil::xit
{
    template <typename T>
    struct Binding // NOLINT
    {
        Frame* frame;
        std::string tag;
        T value;
        std::function<void(const T&)> on_change;
    };

    struct Frame
    {
        Core* core;
        std::string id;
        std::filesystem::path path;

        using Binding_t = std::variant<Binding<std::int64_t>, Binding<std::string>>;
        std::unordered_map<std::string, Binding_t> bindings;
    };

    struct Core
    {
        nil::service::IService* service;
        std::unordered_map<std::string, Frame> frames;
    };
}
