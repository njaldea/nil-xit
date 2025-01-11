#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace nil::xit
{
    template <typename T>
    struct setter
    {
        using type = T;
    };

    template <>
    struct setter<std::string>
    {
        using type = std::string_view;
    };

    template <>
    struct setter<std::vector<std::uint8_t>>
    {
        using type = std::span<const std::uint8_t>;
    };

    template <typename T>
    using setter_t = typename setter<T>::type;
}
