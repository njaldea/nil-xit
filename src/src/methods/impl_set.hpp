#pragma once

#include "../structs.hpp"

#include "../proto/message.pb.h"

namespace nil::xit::impl
{
    // clang-format off
    void msg_set(const char* tag, proto::Binding& msg, bool value);
    void msg_set(const char* tag, proto::Binding& msg, double value);
    void msg_set(const char* tag, proto::Binding& msg, std::int64_t value);
    void msg_set(const char* tag, proto::Binding& msg, std::string value);
    void msg_set(const char* tag, proto::Binding& msg, std::vector<std::uint8_t> value);
    // clang-format on

    // clang-format off
    void msg_set(const char* tag, proto::Binding& msg, const Binding<bool>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const Binding<double>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const Binding<std::int64_t>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const Binding<std::string>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const Binding<std::vector<std::uint8_t>>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<bool>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<double>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<std::int64_t>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<std::string>& binding);
    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<std::vector<std::uint8_t>>& binding);
    // clang-format on

    // clang-format off
    void binding_set(const char* tag, Binding<bool>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, Binding<double>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, Binding<std::int64_t>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, Binding<std::string>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, Binding<std::vector<std::uint8_t>>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, TaggedBinding<bool>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, TaggedBinding<double>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, TaggedBinding<std::int64_t>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, TaggedBinding<std::string>& binding, const proto::Binding& msg);
    void binding_set(const char* tag, TaggedBinding<std::vector<std::uint8_t>>& binding, const proto::Binding& msg);
    // clang-format on

    // clang-format off
    void msg_set(proto::Listener& msg, const Listener<void>& listener);
    void msg_set(proto::Listener& msg, const Listener<bool>& listener);
    void msg_set(proto::Listener& msg, const Listener<double>& listener);
    void msg_set(proto::Listener& msg, const Listener<std::int64_t>& listener);
    void msg_set(proto::Listener& msg, const Listener<std::string_view>& listener);
    void msg_set(proto::Listener& msg, const Listener<std::span<const std::uint8_t>>& listener);
    void msg_set(proto::Listener& msg, const TaggedListener<void>& listener);
    void msg_set(proto::Listener& msg, const TaggedListener<bool>& listener);
    void msg_set(proto::Listener& msg, const TaggedListener<double>& listener);
    void msg_set(proto::Listener& msg, const TaggedListener<std::int64_t>& listener);
    void msg_set(proto::Listener& msg, const TaggedListener<std::string_view>& listener);
    void msg_set(proto::Listener& msg, const TaggedListener<std::span<const std::uint8_t>>& listener);
    // clang-format on

    // clang-format off
    void invoke(const char* tag, const Listener<void>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const Listener<bool>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const Listener<double>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const Listener<std::int64_t>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const Listener<std::string_view>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const Listener<std::span<const std::uint8_t>>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const TaggedListener<void>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const TaggedListener<bool>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const TaggedListener<double>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const TaggedListener<std::int64_t>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const TaggedListener<std::string_view>& listener, const proto::ListenerNotify& msg);
    void invoke(const char* tag, const TaggedListener<std::span<const std::uint8_t>>& listener, const proto::ListenerNotify& msg);
    // clang-format on
}
