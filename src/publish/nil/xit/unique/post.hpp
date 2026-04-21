#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <type_traits>
#include <vector>

namespace nil::xit::unique
{
    namespace impl
    {
        void post(
            const Value<std::vector<std::uint8_t>>& value,
            std::vector<std::uint8_t> new_value
        );
    }

    template <typename T>
        requires(detail::is_built_in_value<T>)
    void post(const Value<T>& value, const typename std::type_identity_t<T>& new_value)
    {
        impl::post(value, std::move(new_value));
    }

    template <detail::has_serialize T>
        requires(!detail::is_built_in_value<T>)
    void post(const Value<T>& value, const std::type_identity_t<T>& new_value)
    {
        impl::post(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<const Value<std::vector<std::uint8_t>>&>(value),
            detail::buffer_type<T>::serialize(std::move(new_value))
        );
    }
}
