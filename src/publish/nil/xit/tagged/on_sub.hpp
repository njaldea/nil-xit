#pragma once

#include "structs.hpp"

#include <functional>

namespace nil::xit::tagged
{
    void on_sub(Frame& frame, std::function<void(std::string_view, std::size_t)> callback);
}
