#pragma once

#include "structs.hpp"

#include "../utils.hpp"

#include "../proto/message.pb.h"

#include <type_traits>

namespace nil::xit::tagged
{
    template <typename T>
    void msg_set(const Value<T>& value, proto::Value& msg, const char* tag)
    {
        nil::xit::utils::msg_set(value.getter(tag), msg);
    }

    template <typename T>
    void value_set(Value<T>& value, const proto::Value& msg, const char* tag)
    {
        if (value.setter)
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                value.setter(tag, msg.value_boolean());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                value.setter(tag, msg.value_double());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                value.setter(tag, msg.value_number());
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                value.setter(tag, msg.value_string());
            }
            else if constexpr (std::is_same_v<T, std::vector<std::int8_t>>)
            {
                const auto& buffer = msg.value_buffer();
                const auto span = std::span<const std::uint8_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    reinterpret_cast<const std::uint8_t*>(buffer.data()),
                    buffer.size()
                );
                value.setter(tag, span);
            }
        }
    }

    template <typename T>
    void invoke(const Signal<T>& signal, const proto::SignalNotify& msg, const char* tag)
    {
        if (signal.on_call)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                signal.on_call(tag);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                signal.on_call(tag, msg.arg_boolean());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                signal.on_call(tag, msg.arg_double());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                signal.on_call(tag, msg.arg_number());
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                signal.on_call(tag, msg.arg_string());
            }
            else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
            {
                const auto& buffer = msg.arg_buffer();
                const auto span = std::span<const std::uint8_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    reinterpret_cast<const std::uint8_t*>(buffer.data()),
                    buffer.size()
                );
                signal.on_call(tag, span);
            }
        }
    }
}
