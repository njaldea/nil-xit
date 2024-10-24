#pragma once

#include "structs.hpp"

#include "../utils.hpp"

#include "../proto/message.pb.h"

#include <type_traits>

namespace nil::xit::unique
{
    template <typename T>
    void msg_set(const Value<T>& value, proto::Value& msg, const char* /* tag */)
    {
        nil::xit::utils::msg_set(value.value, msg);
    }

    template <typename T>
    void value_set(Value<T>& value, const proto::Value& msg, const char* /* tag */)
    {
        constexpr auto apply = [](Value<T>& v, const proto::Value& m)
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
                return set(v.value, m.value_boolean());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return set(v.value, m.value_double());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                return set(v.value, m.value_number());
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return set(v.value, m.value_string());
            }
            else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
            {
                const auto& mm = m.value_buffer();
                const auto& vv = v.value;
                if (vv.size() != mm.size() || 0 != std::memcmp(vv.data(), mm.data(), vv.size()))
                {
                    v.value = {m.value_buffer().begin(), m.value_buffer().end()};
                    return true;
                }
                return false;
            }
        };

        if (apply(value, msg) && value.on_change)
        {
            value.on_change(value.value);
        }
    }

    template <typename T>
    void invoke(const Signal<T>& signal, const proto::SignalNotify& msg, const char* /* tag */)
    {
        if (signal.on_call)
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
                signal.on_call({reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()}
                );
            }
        }
    }
}
