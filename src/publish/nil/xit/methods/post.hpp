#pragma once

#include "../structs.hpp"

#include <string>
#include <vector>

namespace nil::xit
{
    // clang-format off
    void post(const Binding<bool>& binding, bool value);
    void post(const Binding<double>& binding, double value);
    void post(const Binding<std::int64_t>& binding, std::int64_t value);
    void post(const Binding<std::string>& binding, std::string value);
    void post(const Binding<std::vector<std::uint8_t>>& binding, std::vector<std::uint8_t> value);

    template <typename T>
    void post(const Binding<T>& binding, T value)
    {
        post(
            // NOLINTNEXTLINE
            reinterpret_cast<const Binding<std::vector<std::uint8_t>>&>(binding),
            buffer_type<T>::serialize(std::move(value))
        );
    }

    void post(std::string_view, const TaggedBinding<bool>& binding, bool value);
    void post(std::string_view, const TaggedBinding<double>& binding, double value);
    void post(std::string_view, const TaggedBinding<std::int64_t>& binding, std::int64_t value);
    void post(std::string_view, const TaggedBinding<std::string>& binding, std::string value);
    void post(std::string_view, const TaggedBinding<std::vector<std::uint8_t>>& binding, std::vector<std::uint8_t> value);
    
    template <typename T>
    void post(std::string_view tag, const TaggedBinding<T>& binding, T value)
    {
        post(
            tag,
            // NOLINTNEXTLINE
            reinterpret_cast<const TaggedBinding<std::vector<std::uint8_t>>&>(binding),
            buffer_type<T>::serialize(std::move(value))
        );
    }

    // clang-format on
}
