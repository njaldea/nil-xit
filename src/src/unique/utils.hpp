#pragma once

#include "../structs.hpp"

#include "../proto/message.pb.h"

namespace nil::xit::unique::impl
{
    // clang-format off
    void msg_set(const Binding<bool>& binding, proto::Binding& msg, const char* tag);
    void msg_set(const Binding<double>& binding, proto::Binding& msg, const char* tag);
    void msg_set(const Binding<std::int64_t>& binding, proto::Binding& msg, const char* tag);
    void msg_set(const Binding<std::string>& binding, proto::Binding& msg, const char* tag);
    void msg_set(const Binding<std::vector<std::uint8_t>>& binding, proto::Binding& msg, const char* tag);

    void binding_set(Binding<bool>& binding, const proto::Binding& msg, const char* tag);
    void binding_set(Binding<double>& binding, const proto::Binding& msg, const char* tag);
    void binding_set(Binding<std::int64_t>& binding, const proto::Binding& msg, const char* tag);
    void binding_set(Binding<std::string>& binding, const proto::Binding& msg, const char* tag);
    void binding_set(Binding<std::vector<std::uint8_t>>& binding, const proto::Binding& msg, const char* tag);

    void msg_set(const Listener<void>& listener, proto::Listener& msg);
    void msg_set(const Listener<bool>& listener, proto::Listener& msg);
    void msg_set(const Listener<double>& listener, proto::Listener& msg);
    void msg_set(const Listener<std::int64_t>& listener, proto::Listener& msg);
    void msg_set(const Listener<std::string_view>& listener, proto::Listener& msg);
    void msg_set(const Listener<std::span<const std::uint8_t>>& listener, proto::Listener& msg);

    void invoke(const Listener<void>& listener, const proto::ListenerNotify& msg, const char* tag);
    void invoke(const Listener<bool>& listener, const proto::ListenerNotify& msg, const char* tag);
    void invoke(const Listener<double>& listener, const proto::ListenerNotify& msg, const char* tag);
    void invoke(const Listener<std::int64_t>& listener, const proto::ListenerNotify& msg, const char* tag);
    void invoke(const Listener<std::string_view>& listener, const proto::ListenerNotify& msg, const char* tag);
    void invoke(const Listener<std::span<const std::uint8_t>>& listener, const proto::ListenerNotify& msg, const char* tag);
    // clang-format on
}
