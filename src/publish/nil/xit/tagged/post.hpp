#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <string>
#include <vector>

namespace nil::xit::tagged
{
    namespace impl
    {
        void post(
            const Value<std::vector<std::uint8_t>>& value,
            std::string tag,
            std::vector<std::uint8_t> new_value
        );
    }

    template <typename T>
        requires(is_built_in_value<T>)
    void post(const Value<T>& value, std::string tag, const std::type_identity_t<T>& new_value)
    {
        impl::post(value, std::move(tag), std::move(new_value));
    }

    template <has_serialize T>
        requires(!is_built_in_value<T>)
    void post(const Value<T>& value, std::string tag, const std::type_identity_t<T>& new_value)
    {
        post(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<const Value<std::vector<std::uint8_t>>&>(value),
            std::move(tag),
            buffer_type<T>::serialize(std::move(new_value))
        );
    }
}
