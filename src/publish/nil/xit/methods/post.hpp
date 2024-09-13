#pragma once

#include "../structs.hpp"

#include <string>
#include <vector>

namespace nil::xit
{
    void post(const Binding<bool>& binding, bool value);
    void post(const Binding<double>& binding, double value);
    void post(const Binding<std::int64_t>& binding, std::int64_t value);
    void post(const Binding<std::string>& binding, std::string value);
    void post(const Binding<std::vector<std::uint8_t>>& binding, std::vector<std::uint8_t> value);

    template <typename T>
    void post(const Binding<T>& binding, T value)
    {
        post( // NOLINTNEXTLINE
            reinterpret_cast<const Binding<std::vector<std::uint8_t>>&>(binding),
            buffer_type<T>::serialize(std::move(value))
        );
    }
}
