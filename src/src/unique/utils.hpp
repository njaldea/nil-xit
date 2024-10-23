#pragma once

#include "structs.hpp"

#include "../utils.hpp"

#include "../proto/message.pb.h"

#include <type_traits>

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T>
        bool value_set(Value<T>& value, const proto::Value& msg)
        {
            constexpr auto set = [](T& l, auto&& r)
            {
                if (l != r)
                {
                    l = std::forward<decltype(r)>(r);
                    return true;
                }
                return false;
            };
            if constexpr (std::is_same_v<T, bool>)
            {
                return set(value.value, msg.value_boolean());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return set(value.value, msg.value_double());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                return set(value.value, msg.value_number());
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return set(value.value, msg.value_string());
            }
            else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
            {
                const auto size = value.value.size();
                const auto is_size_equal = size == msg.value_buffer().size();
                if (is_size_equal)
                {
                    return false;
                }
                const auto* value_data = value.value.data();
                const auto* msg_data = msg.value_buffer().data();
                const auto is_content_equal = 0 == std::memcmp(value_data, msg_data, size);
                if (is_content_equal)
                {
                    return false;
                }
                value.value = {msg.value_buffer().begin(), msg.value_buffer().end()};
                return true;
            }
        }
    }

    template <typename T>
    void msg_set(const Value<T>& value, proto::Value& msg, const char* /* tag */)
    {
        nil::xit::utils::msg_set(value.value, msg);
    }

    template <typename T>
    void value_set(Value<T>& value, const proto::Value& msg, const char* /* tag */)
    {
        if (impl::value_set(value, msg) && value.on_change)
        {
            value.on_change(value.value);
        }
    }

    template <typename T>
    void invoke(const Signal<T>& signal, const proto::SignalNotify& msg, const char* /* tag */)
    {
        if constexpr (std::is_same_v<T, void>)
        {
            (void)msg;
            signal.on_call();
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            signal.on_call(msg.arg_boolean());
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            signal.on_call(msg.arg_double());
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            signal.on_call(msg.arg_number());
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            signal.on_call(msg.arg_string());
        }
        else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
        {
            const auto& buffer = msg.arg_buffer();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            signal.on_call({reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()});
        }
    }
}
