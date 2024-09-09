#pragma once

#include "../structs.hpp"

#include <string>

namespace nil::xit
{
    void post(Binding<std::int64_t>& binding, std::int64_t value);
    void post(Binding<std::string>& binding, std::string value);
}
