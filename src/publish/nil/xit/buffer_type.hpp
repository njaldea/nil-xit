#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
#include <vector>

#include <nil/service/codec.hpp>

template <typename T>
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
struct nil::service::codec<T>
{
    static std::size_t size(const T& message)
    {
        return message.size();
    }

    static std::size_t serialize(void* output, const T& data)
    {
        std::memcpy(static_cast<char*>(output), data.data(), data.size());
        return data.size();
    }

    static T deserialize(const void* input, std::uint64_t size)
    {
        return T(static_cast<const char*>(input), size);
    }
};

namespace nil::xit
{
    namespace detail
    {
        template <typename T>
        concept service_codec_has_serialize = requires() {
            nil::service::codec<T>::size;
            nil::service::codec<T>::serialize;
        };

        template <typename T>
        concept service_codec_has_derialize = requires() { nil::service::codec<T>::deserialize; };

        template <service_codec_has_derialize T>
        static T deserialize(const void* data, std::uint64_t size)
        {
            return nil::service::codec<T>::deserialize(data, size);
        }

        template <service_codec_has_serialize T>
        static std::vector<std::uint8_t> serialize(const T& value)
        {
            std::vector<std::uint8_t> buffer(nil::service::codec<T>::size(value));
            nil::service::codec<T>::serialize(buffer.data(), value);
            return buffer;
        }

        template <typename T>
        struct resolver
        {
            using type = T;
        };

        template <typename T>
            requires std::is_floating_point_v<T>
        struct resolver<T>
        {
            using type = double;
        };

        template <typename T>
            requires std::is_integral_v<T>
        struct resolver<T>
        {
            using type = std::int64_t;
        };
    }

    template <typename T>
    struct buffer_type
    {
        static T deserialize(const void* data, std::uint64_t size)
            requires detail::service_codec_has_derialize<T>
        {
            return nil::service::codec<typename detail::resolver<T>::type>::deserialize(data, size);
        }

        static std::vector<std::uint8_t> serialize(const T& value)
            requires detail::service_codec_has_serialize<T>
        {
            std::vector<std::uint8_t> buffer(
                nil::service::codec<typename detail::resolver<T>::type>::size(value)
            );
            nil::service::codec<typename detail::resolver<T>::type>::serialize(
                buffer.data(),
                value
            );
            return buffer;
        }
    };

    template <typename T>
    concept is_built_in_value = std::is_same_v<T, std::vector<std::uint8_t>>;

    template <typename T>
    concept is_built_in_signal = std::is_same_v<T, std::span<const std::uint8_t>>;

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
