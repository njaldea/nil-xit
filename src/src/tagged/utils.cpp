#include "utils.hpp"
#include "../utils.hpp"

#include <type_traits>

namespace nil::xit::tagged
{
    void msg_set(const Value<bool>& value, proto::Value& msg, const char* tag)
    {
        nil::xit::impl::msg_set(value.getter(tag), msg, tag);
    }

    void msg_set(const Value<double>& value, proto::Value& msg, const char* tag)
    {
        nil::xit::impl::msg_set(value.getter(tag), msg, tag);
    }

    void msg_set(const Value<std::int64_t>& value, proto::Value& msg, const char* tag)
    {
        nil::xit::impl::msg_set(value.getter(tag), msg, tag);
    }

    void msg_set(const Value<std::string>& value, proto::Value& msg, const char* tag)
    {
        nil::xit::impl::msg_set(value.getter(tag), msg, tag);
    }

    void msg_set(const Value<std::vector<std::uint8_t>>& value, proto::Value& msg, const char* tag)
    {
        nil::xit::impl::msg_set(value.getter(tag), msg, tag);
    }

    namespace impl
    {
        template <typename T>
        bool value_set(std::decay_t<T>& value_out, T&& value_in)
        {
            if (value_out != value_in)
            {
                value_out = std::forward<T>(value_in);
                return true;
            }
            return false;
        }
    }

    void value_set(Value<bool>& value, const proto::Value& msg, const char* tag)
    {
        if (value.setter)
        {
            value.setter(tag, msg.value_boolean());
        }
    }

    void value_set(Value<double>& value, const proto::Value& msg, const char* tag)
    {
        if (value.setter)
        {
            value.setter(tag, msg.value_double());
        }
    }

    void value_set(Value<std::int64_t>& value, const proto::Value& msg, const char* tag)
    {
        if (value.setter)
        {
            value.setter(tag, msg.value_number());
        }
    }

    void value_set(Value<std::string>& value, const proto::Value& msg, const char* tag)
    {
        if (value.setter)
        {
            value.setter(tag, msg.value_string());
        }
    }

    void value_set(
        Value<std::vector<std::uint8_t>>& value,
        const proto::Value& msg,
        const char* tag
    )
    {
        if (value.setter)
        {
            const auto& buffer = msg.value_buffer();
            value.setter(
                tag,
                // NOLINTNEXTLINE
                {reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()}
            );
        }
    }

    void msg_set(const Signal<void>& signal, proto::Signal& msg)
    {
        (void)msg;
        (void)signal;
    }

    void msg_set(const Signal<bool>& signal, proto::Signal& msg)
    {
        (void)signal;
        msg.set_type("arg_boolean");
    }

    void msg_set(const Signal<double>& signal, proto::Signal& msg)
    {
        (void)signal;
        msg.set_type("arg_double");
    }

    void msg_set(const Signal<std::int64_t>& signal, proto::Signal& msg)
    {
        (void)signal;
        msg.set_type("arg_number");
    }

    void msg_set(const Signal<std::string_view>& signal, proto::Signal& msg)
    {
        (void)signal;
        msg.set_type("arg_string");
    }

    void msg_set(const Signal<std::span<const std::uint8_t>>& signal, proto::Signal& msg)
    {
        (void)signal;
        msg.set_type("arg_buffer");
    }

    void invoke(const Signal<void>& signal, const proto::SignalNotify& msg, const char* tag)
    {
        (void)msg;
        signal.on_change(tag);
    }

    void invoke(const Signal<bool>& signal, const proto::SignalNotify& msg, const char* tag)
    {
        signal.on_change(tag, msg.arg_boolean());
    }

    void invoke(const Signal<double>& signal, const proto::SignalNotify& msg, const char* tag)
    {
        signal.on_change(tag, msg.arg_double());
    }

    void invoke(const Signal<std::int64_t>& signal, const proto::SignalNotify& msg, const char* tag)
    {
        signal.on_change(tag, msg.arg_number());
    }

    void invoke(
        const Signal<std::string_view>& signal,
        const proto::SignalNotify& msg,
        const char* tag
    )
    {
        signal.on_change(tag, msg.arg_string());
    }

    void invoke(
        const Signal<std::span<const std::uint8_t>>& signal,
        const proto::SignalNotify& msg,
        const char* tag
    )
    {
        const auto& buffer = msg.arg_buffer();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        signal.on_change(
            tag,
            // NOLINTNEXTLINE
            {reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()}
        );
    }
}
