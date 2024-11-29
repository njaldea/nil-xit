#pragma once

#include "structs.hpp"

namespace nil::xit::tagged
{
    bool has_subscribers(const Frame& frame, std::string_view tag);
}
