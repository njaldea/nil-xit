#pragma once

#include "structs.hpp"

#include <functional>

namespace nil::xit::tagged
{
    void on_load(Frame& frame, std::function<void(std::string_view)> callback);
}
