#pragma once

#include "../structs.hpp"

#include <string>
#include <vector>

namespace nil::xit
{
    void set(Binding<std::int64_t>& binding, std::int64_t value);
    void set(Binding<std::string>& binding, std::string value);
    void set(Binding<std::vector<std::uint8_t>>& binding, std::vector<std::uint8_t> value);
}
