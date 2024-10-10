#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <string>
#include <vector>

namespace nil::xit::unique
{
    void post(const Value<bool>& value, bool new_value);
    void post(const Value<double>& value, double new_value);
    void post(const Value<std::int64_t>& value, std::int64_t new_value);
    void post(const Value<std::string>& value, std::string new_value);
    void post(const Value<std::vector<std::uint8_t>>& value, std::vector<std::uint8_t> new_value);

    template <typename T>
    void post(const Value<T>& value, const T& new_value)
    {
        post(
            // NOLINTNEXTLINE
            reinterpret_cast<const Value<std::vector<std::uint8_t>>&>(value),
            buffer_type<T>::serialize(std::move(new_value))
        );
    }
}
