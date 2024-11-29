#pragma once

#include "structs.hpp"

#include <functional>

namespace nil::xit::unique
{
    void on_load(Frame& frame, std::function<void()> callback);
}
