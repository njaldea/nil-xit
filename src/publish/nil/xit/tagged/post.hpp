#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <string>
#include <vector>

namespace nil::xit::tagged
{
    void post(const Value<bool>& value, std::string, bool new_value);
    void post(const Value<double>& value, std::string, double new_value);
    void post(const Value<std::int64_t>& value, std::string, std::int64_t new_value);
    void post(const Value<std::string>& value, std::string, std::string new_value);
    void post(
        const Value<std::vector<std::uint8_t>>& value,
        std::string,
        std::vector<std::uint8_t> new_value
    );

    template <typename T>
        requires(!is_built_in_value<T>)
    void post(const Value<T>& value, std::string tag, const T& new_value)
    {
        static_assert(has_serialize<T>, "requires buffer_type<T>::serialize");
        post(
            // NOLINTNEXTLINE
            reinterpret_cast<const Value<std::vector<std::uint8_t>>&>(value),
            std::move(tag),
            buffer_type<T>::serialize(new_value)
        );
    }
}
