#pragma once

#include <cstdint>
#include <vector>

namespace nil::xit
{
    template <typename T>
    struct buffer_type
    {
        static T deserialize(const void* data, std::uint64_t size) = delete;
        static std::vector<std::uint8_t> serialize(const T& value) = delete;
    };
}
