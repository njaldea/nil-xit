#pragma once

#include "../structs.hpp"

#include <functional>
#include <string>

namespace nil::xit
{
    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string tag,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change = {}
    );
    Binding<std::string>& bind(
        Frame& frame,
        std::string tag,
        std::string value,
        std::function<void(const std::string&)> on_change = {}
    );
}
