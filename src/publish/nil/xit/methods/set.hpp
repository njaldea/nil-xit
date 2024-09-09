#pragma once

#include "../structs.hpp"

#include <string>

namespace nil::xit
{
    void set(Binding<std::int64_t>& binding, std::int64_t value);
    void set(Binding<std::string>& binding, std::string value);
}
