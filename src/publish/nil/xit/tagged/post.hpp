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
    void post(std::string_view, const Value<std::string>& value, std::string new_value);
    void post(
        std::string_view,
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value
    );

    template <typename T>
        requires(!is_built_in_value<T>)
    void post(std::string_view tag, const Value<T>& value, const T& new_value)
    {
        static_assert(has_serialize<T>, "requires buffer_type<T>::serialize");
        post(
            tag,
            // NOLINTNEXTLINE
            reinterpret_cast<const Value<std::vector<std::uint8_t>>&>(value),
            buffer_type<T>::serialize(new_value)
        );
    }
}
