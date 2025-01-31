#pragma once

#include "messages/message.pb.h"

#include <type_traits>

namespace nil::xit::utils
{
    template <typename T>
    void unreachable();

    namespace transparent
    {
        struct Hash
        {
            using is_transparent = void;

            std::size_t operator()(std::string_view s) const
            {
                return std::hash<std::string_view>()(s);
            }
        };

        struct Equal
        {
            using is_transparent = void;

            bool operator()(std::string_view l, std::string_view r) const
            {
                return l == r;
            }
        };

        template <typename T>
        using hash_map = std::unordered_map<std::string, T, Hash, Equal>;
    }

    template <typename T>
    void msg_set(const T& value, proto::Value& msg)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            msg.set_value_boolean(value);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            msg.set_value_double(value);
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            msg.set_value_number(value);
        }
        else if constexpr (std::is_same_v<T, std::string_view>)
        {
            msg.set_value_string(value);
        }
        else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
        {
            msg.set_value_buffer(value.data(), value.size());
        }
        else
        {
            unreachable<T>();
        }
    }

    template <template <class> class Signal, typename T>
    void msg_set(const Signal<T>& /* signal */, proto::Signal& msg)
    {
        if constexpr (std::is_same_v<T, void>)
        {
            (void)msg;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            msg.set_type("arg_boolean");
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            msg.set_type("arg_double");
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            msg.set_type("arg_number");
        }
        else if constexpr (std::is_same_v<T, std::string_view>)
        {
            msg.set_type("arg_string");
        }
        else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
        {
            msg.set_type("arg_buffer");
        }
        else
        {
            unreachable<T>();
        }
    }
}
