#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <string>
#include <vector>

namespace nil::xit::tagged
{
    void post(std::string_view, const Value<bool>& value, bool new_value);
    void post(std::string_view, const Value<double>& value, double new_value);
    void post(std::string_view, const Value<std::int64_t>& value, std::int64_t new_value);
    void post(std::string_view, const Value<std::string>& value, std::string_view new_value);
    void post(
        std::string_view,
        const Value<std::vector<std::uint8_t>>& value,
        std::span<const std::uint8_t> new_value
    );

    template <has_serialize T>
    void post(std::string_view tag, const Value<T>& value, const T& new_value)
    {
        post(
            tag,
            // NOLINTNEXTLINE
            reinterpret_cast<const Value<std::vector<std::uint8_t>>&>(value),
            buffer_type<T>::serialize(new_value)
        );
    }
}
