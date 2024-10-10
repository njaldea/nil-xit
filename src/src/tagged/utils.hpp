#pragma once

#include "structs.hpp"

#include "../proto/message.pb.h"

namespace nil::xit::tagged::impl
{
    // clang-format off
    void msg_set(const Value<bool>& value, proto::Value& msg, const char* tag);
    void msg_set(const Value<double>& value, proto::Value& msg, const char* tag);
    void msg_set(const Value<std::int64_t>& value, proto::Value& msg, const char* tag);
    void msg_set(const Value<std::string>& value, proto::Value& msg, const char* tag);
    void msg_set(const Value<std::vector<std::uint8_t>>& value, proto::Value& msg, const char* tag);

    void value_set(Value<bool>& value, const proto::Value& msg, const char* tag);
    void value_set(Value<double>& value, const proto::Value& msg, const char* tag);
    void value_set(Value<std::int64_t>& value, const proto::Value& msg, const char* tag);
    void value_set(Value<std::string>& value, const proto::Value& msg, const char* tag);
    void value_set(Value<std::vector<std::uint8_t>>& value, const proto::Value& msg, const char* tag);

    void msg_set(const Signal<void>& signal, proto::Signal& msg);
    void msg_set(const Signal<bool>& signal, proto::Signal& msg);
    void msg_set(const Signal<double>& signal, proto::Signal& msg);
    void msg_set(const Signal<std::int64_t>& signal, proto::Signal& msg);
    void msg_set(const Signal<std::string_view>& signal, proto::Signal& msg);
    void msg_set(const Signal<std::span<const std::uint8_t>>& signal, proto::Signal& msg);

    void invoke(const Signal<void>& signal, const proto::SignalNotify& msg, const char* tag);
    void invoke(const Signal<bool>& signal, const proto::SignalNotify& msg, const char* tag);
    void invoke(const Signal<double>& signal, const proto::SignalNotify& msg, const char* tag);
    void invoke(const Signal<std::int64_t>& signal, const proto::SignalNotify& msg, const char* tag);
    void invoke(const Signal<std::string_view>& signal, const proto::SignalNotify& msg, const char* tag);
    void invoke(const Signal<std::span<const std::uint8_t>>& signal, const proto::SignalNotify& msg, const char* tag);
    // clang-format on
}
