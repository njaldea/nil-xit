#pragma once

#include <nil/xit/structs.hpp>

#include "Binding.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>

namespace nil::xit
{
    struct Frame
    {
        Core* core;
        std::string id;
        std::filesystem::path path;

        using Binding_t = std::variant<Binding<std::int64_t>, Binding<std::string>>;
        std::unordered_map<std::string, Binding_t> bindings;
    };
}
