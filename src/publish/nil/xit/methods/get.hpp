#pragma once

#include "../structs.hpp"

#include <string>
#include <vector>

namespace nil::xit
{
    bool get(const Binding<bool>& binding);
    double get(const Binding<double>& binding);
    std::int64_t get(const Binding<std::int64_t>& binding);
    std::string get(const Binding<std::string>& binding);
    std::vector<std::uint8_t> get(const Binding<std::vector<std::uint8_t>>& binding);

    template <typename T>
    T get(const Binding<T>& binding)
    {
        auto buffer = get( // NOLINTNEXTLINE
            reinterpret_cast<const Binding<std::vector<std::uint8_t>>&>(binding)
        );
        return deserialize<T>(buffer.data(), buffer.size());
    }
}
