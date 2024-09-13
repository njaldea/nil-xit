#include "impl_set.hpp"
#include <type_traits>

namespace nil::xit::impl
{

    void msg_set(nil::xit::proto::Binding& msg, bool value)
    {
        msg.set_value_boolean(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, double value)
    {
        msg.set_value_double(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, std::int64_t value)
    {
        msg.set_value_number(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, const std::string& value)
    {
        msg.set_value_string(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, const std::vector<std::uint8_t>& value)
    {
        msg.set_value_buffer(value.data(), value.size());
    }

    namespace impl
    {
        template <typename T>
        bool binding_set(std::decay_t<T>& value_out, T&& value_in)
        {
            if (value_out != value_in)
            {
                value_out = std::forward<T>(value_in);
                return true;
            }
            return false;
        }
    }

    bool binding_set(bool& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(value, msg.value_boolean());
    }

    bool binding_set(double& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(value, msg.value_double());
    }

    bool binding_set(std::int64_t& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(value, msg.value_number());
    }

    bool binding_set(std::string& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(value, msg.value_string());
    }

    bool binding_set(std::vector<std::uint8_t>& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(
            value,
            std::vector<std::uint8_t>(msg.value_buffer().begin(), msg.value_buffer().end())
        );
    }

    void msg_set(nil::xit::proto::Listener& msg, const Listener<void>& listener)
    {
        (void)msg;
        (void)listener;
    }

    void msg_set(nil::xit::proto::Listener& msg, const Listener<bool>& listener)
    {
        (void)listener;
        msg.set_type("arg_boolean");
    }

    void msg_set(nil::xit::proto::Listener& msg, const Listener<double>& listener)
    {
        (void)listener;
        msg.set_type("arg_double");
    }

    void msg_set(nil::xit::proto::Listener& msg, const Listener<std::int64_t>& listener)
    {
        (void)listener;
        msg.set_type("arg_number");
    }

    void msg_set(nil::xit::proto::Listener& msg, const Listener<std::string>& listener)
    {
        (void)listener;
        msg.set_type("arg_string");
    }

    void msg_set(
        nil::xit::proto::Listener& msg,
        const Listener<std::span<const std::uint8_t>>& listener
    )
    {
        (void)listener;
        msg.set_type("arg_buffer");
    }

    void invoke(const Listener<void>& listener, const nil::xit::proto::ListenerNotify& msg)
    {
        (void)msg;
        listener.on_change();
    }

    void invoke(const Listener<bool>& listener, const nil::xit::proto::ListenerNotify& msg)
    {
        listener.on_change(msg.arg_boolean());
    }

    void invoke(const Listener<double>& listener, const nil::xit::proto::ListenerNotify& msg)
    {
        listener.on_change(msg.arg_double());
    }

    void invoke(const Listener<std::int64_t>& listener, const nil::xit::proto::ListenerNotify& msg)
    {
        listener.on_change(msg.arg_number());
    }

    void invoke(const Listener<std::string>& listener, const nil::xit::proto::ListenerNotify& msg)
    {
        listener.on_change(msg.arg_string());
    }

    void invoke(
        const Listener<std::span<const std::uint8_t>>& listener,
        const nil::xit::proto::ListenerNotify& msg
    )
    {
        // NOLINTNEXTLINE
        auto& buffer = msg.arg_buffer();
        // NOLINTNEXTLINE
        listener.on_change({reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()});
    }
}
