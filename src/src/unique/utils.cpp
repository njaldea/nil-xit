#include "utils.hpp"

#include "../utils.hpp"

#include <type_traits>

namespace nil::xit::unique::impl
{
    void msg_set(const Binding<bool>& binding, proto::Binding& msg, const char* tag)
    {
        nil::xit::impl::msg_set(binding.value, msg, tag);
    }

    void msg_set(const Binding<double>& binding, proto::Binding& msg, const char* tag)
    {
        nil::xit::impl::msg_set(binding.value, msg, tag);
    }

    void msg_set(const Binding<std::int64_t>& binding, proto::Binding& msg, const char* tag)
    {
        nil::xit::impl::msg_set(binding.value, msg, tag);
    }

    void msg_set(const Binding<std::string>& binding, proto::Binding& msg, const char* tag)
    {
        nil::xit::impl::msg_set(binding.value, msg, tag);
    }

    void msg_set(
        const Binding<std::vector<std::uint8_t>>& binding,
        proto::Binding& msg,
        const char* tag
    )
    {
        nil::xit::impl::msg_set(binding.value, msg, tag);
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

    void binding_set(Binding<bool>& binding, const proto::Binding& msg, const char* tag)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_boolean()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(Binding<double>& binding, const proto::Binding& msg, const char* tag)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_double()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(Binding<std::int64_t>& binding, const proto::Binding& msg, const char* tag)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_number()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(Binding<std::string>& binding, const proto::Binding& msg, const char* tag)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_string()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(
        Binding<std::vector<std::uint8_t>>& binding,
        const proto::Binding& msg,
        const char* tag
    )
    {
        (void)tag;
        auto& value = binding.value;
        if (value.size() != msg.value_buffer().size()
            || 0 != std::memcmp(value.data(), msg.value_buffer().data(), value.size()))
        {
            value = {msg.value_buffer().begin(), msg.value_buffer().end()};
            if (binding.on_change)
            {
                binding.on_change(value);
            }
        }
    }

    void msg_set(const Listener<void>& listener, proto::Listener& msg)
    {
        (void)msg;
        (void)listener;
    }

    void msg_set(const Listener<bool>& listener, proto::Listener& msg)
    {
        (void)listener;
        msg.set_type("arg_boolean");
    }

    void msg_set(const Listener<double>& listener, proto::Listener& msg)
    {
        (void)listener;
        msg.set_type("arg_double");
    }

    void msg_set(const Listener<std::int64_t>& listener, proto::Listener& msg)
    {
        (void)listener;
        msg.set_type("arg_number");
    }

    void msg_set(const Listener<std::string_view>& listener, proto::Listener& msg)
    {
        (void)listener;
        msg.set_type("arg_string");
    }

    void msg_set(const Listener<std::span<const std::uint8_t>>& listener, proto::Listener& msg)
    {
        (void)listener;
        msg.set_type("arg_buffer");
    }

    void invoke(const Listener<void>& listener, const proto::ListenerNotify& msg, const char* tag)
    {
        (void)tag;
        (void)msg;
        listener.on_change();
    }

    void invoke(const Listener<bool>& listener, const proto::ListenerNotify& msg, const char* tag)
    {
        (void)tag;
        listener.on_change(msg.arg_boolean());
    }

    void invoke(const Listener<double>& listener, const proto::ListenerNotify& msg, const char* tag)
    {
        (void)tag;
        listener.on_change(msg.arg_double());
    }

    void invoke(
        const Listener<std::int64_t>& listener,
        const proto::ListenerNotify& msg,
        const char* tag
    )
    {
        (void)tag;
        listener.on_change(msg.arg_number());
    }

    void invoke(
        const Listener<std::string_view>& listener,
        const proto::ListenerNotify& msg,
        const char* tag
    )
    {
        (void)tag;
        listener.on_change(msg.arg_string());
    }

    void invoke(
        const Listener<std::span<const std::uint8_t>>& listener,
        const proto::ListenerNotify& msg,
        const char* tag
    )
    {
        (void)tag;
        const auto& buffer = msg.arg_buffer();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        listener.on_change({reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()});
    }
}
