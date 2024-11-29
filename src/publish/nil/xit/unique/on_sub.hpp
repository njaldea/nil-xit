#pragma once

#include "structs.hpp"

#include <functional>

namespace nil::xit::unique
{
    void on_sub(Frame& frame, std::function<void(std::size_t)> callback);
}
