#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <string>
#include <vector>

namespace nil::xit::tagged
{
    void post(std::string_view, const Binding<bool>& binding, bool value);
    void post(std::string_view, const Binding<double>& binding, double value);
    void post(std::string_view, const Binding<std::int64_t>& binding, std::int64_t value);
    void post(std::string_view, const Binding<std::string>& binding, std::string value);
    void post(
        std::string_view,
        const Binding<std::vector<std::uint8_t>>& binding,
        std::vector<std::uint8_t> value
    );

    template <typename T>
    void post(std::string_view tag, const Binding<T>& binding, T value)
    {
        post(
            tag,
            // NOLINTNEXTLINE
            reinterpret_cast<const Binding<std::vector<std::uint8_t>>&>(binding),
            buffer_type<T>::serialize(std::move(value))
        );
    }
}
