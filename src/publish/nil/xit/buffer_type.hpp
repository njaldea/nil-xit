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

    template <typename T>
    concept has_serialize = requires(T value) {
        { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
    };

    template <typename T>
    concept has_deserialize = requires(T value) {
        { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
    };

    template <typename T>
    concept has_codec = has_deserialize<T> && has_serialize<T>;
}
