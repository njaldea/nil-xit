#pragma once

#include <nil/xit/structs.hpp>

#include <functional>
#include <string>

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
}
