#pragma once

#include "../structs.hpp"

#include "../proto/message.pb.h"

namespace nil::xit::impl
{
    void msg_set(proto::Binding& msg, bool value);
    void msg_set(proto::Binding& msg, double value);
    void msg_set(proto::Binding& msg, std::int64_t value);
    void msg_set(proto::Binding& msg, const std::string& value);
    void msg_set(proto::Binding& msg, const std::vector<std::uint8_t>& value);

    bool binding_set(bool& value, const proto::Binding& msg);
    bool binding_set(double& value, const proto::Binding& msg);
    bool binding_set(std::int64_t& value, const proto::Binding& msg);
    bool binding_set(std::string& value, const proto::Binding& msg);
    bool binding_set(std::vector<std::uint8_t>& value, const proto::Binding& msg);

    void msg_set(proto::Listener& msg, const Listener<void>& listener);
    void msg_set(proto::Listener& msg, const Listener<bool>& listener);
    void msg_set(proto::Listener& msg, const Listener<double>& listener);
    void msg_set(proto::Listener& msg, const Listener<std::int64_t>& listener);
    void msg_set(proto::Listener& msg, const Listener<std::string_view>& listener);
    void msg_set(proto::Listener& msg, const Listener<std::span<const std::uint8_t>>& listener);

    void invoke(const Listener<void>& listener, const proto::ListenerNotify& msg);
    void invoke(const Listener<bool>& listener, const proto::ListenerNotify& msg);
    void invoke(const Listener<double>& listener, const proto::ListenerNotify& msg);
    void invoke(const Listener<std::int64_t>& listener, const proto::ListenerNotify& msg);
    void invoke(const Listener<std::string_view>& listener, const proto::ListenerNotify& msg);
    void invoke(
        const Listener<std::span<const std::uint8_t>>& listener,
        const proto::ListenerNotify& msg
    );
}
