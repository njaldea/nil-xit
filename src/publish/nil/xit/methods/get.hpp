#pragma once

#include "../structs.hpp"

#include <string>

namespace nil::xit
{
    std::int64_t get(const Binding<std::int64_t>& binding);
    std::string get(const Binding<std::string>& binding);
}
