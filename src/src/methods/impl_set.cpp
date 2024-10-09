#include "impl_set.hpp"
#include <type_traits>

namespace nil::xit::impl
{
    void msg_set(const char* tag, proto::Binding& msg, bool value)
    {
        (void)tag;
        msg.set_value_boolean(value);
    }

    void msg_set(const char* tag, proto::Binding& msg, double value)
    {
        (void)tag;
        msg.set_value_double(value);
    }

    void msg_set(const char* tag, proto::Binding& msg, std::int64_t value)
    {
        (void)tag;
        msg.set_value_number(value);
    }

    void msg_set(const char* tag, proto::Binding& msg, std::string value)
    {
        (void)tag;
        msg.set_value_string(std::move(value));
    }

    void msg_set(const char* tag, proto::Binding& msg, std::vector<std::uint8_t> value)
    {
        (void)tag;
        msg.set_value_buffer(value.data(), value.size());
    }

    void msg_set(const char* tag, proto::Binding& msg, const Binding<bool>& binding)
    {
        msg_set(tag, msg, binding.value);
    }

    void msg_set(const char* tag, proto::Binding& msg, const Binding<double>& binding)
    {
        msg_set(tag, msg, binding.value);
    }

    void msg_set(const char* tag, proto::Binding& msg, const Binding<std::int64_t>& binding)
    {
        msg_set(tag, msg, binding.value);
    }

    void msg_set(const char* tag, proto::Binding& msg, const Binding<std::string>& binding)
    {
        msg_set(tag, msg, binding.value);
    }

    void msg_set(
        const char* tag,
        proto::Binding& msg,
        const Binding<std::vector<std::uint8_t>>& binding
    )
    {
        msg_set(tag, msg, binding.value);
    }

    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<bool>& binding)
    {
        msg.set_value_boolean(binding.getter(tag));
    }

    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<double>& binding)
    {
        msg.set_value_double(binding.getter(tag));
    }

    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<std::int64_t>& binding)
    {
        msg.set_value_number(binding.getter(tag));
    }

    void msg_set(const char* tag, proto::Binding& msg, const TaggedBinding<std::string>& binding)
    {
        msg.set_value_string(binding.getter(tag));
    }

    void msg_set(
        const char* tag,
        proto::Binding& msg,
        const TaggedBinding<std::vector<std::uint8_t>>& binding
    )
    {
        const auto value = binding.getter(tag);
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

    void binding_set(const char* tag, Binding<bool>& binding, const proto::Binding& msg)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_boolean()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(const char* tag, Binding<double>& binding, const proto::Binding& msg)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_double()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(const char* tag, Binding<std::int64_t>& binding, const proto::Binding& msg)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_number()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(const char* tag, Binding<std::string>& binding, const proto::Binding& msg)
    {
        (void)tag;
        if (impl::binding_set(binding.value, msg.value_string()) && binding.on_change)
        {
            binding.on_change(binding.value);
        }
    }

    void binding_set(
        const char* tag,
        Binding<std::vector<std::uint8_t>>& binding,
        const proto::Binding& msg
    )
    {
        (void)tag;
        auto& value = binding.value;
        if (value.size() == msg.value_buffer().size()
            && 0 == std::memcmp(value.data(), msg.value_buffer().data(), value.size()))
        {
            value = {msg.value_buffer().begin(), msg.value_buffer().end()};
            if (binding.on_change)
            {
                binding.on_change(binding.value);
            }
        }
    }

    void binding_set(const char* tag, TaggedBinding<bool>& binding, const proto::Binding& msg)
    {
        if (binding.setter)
        {
            binding.setter(tag, msg.value_boolean());
        }
    }

    void binding_set(const char* tag, TaggedBinding<double>& binding, const proto::Binding& msg)
    {
        if (binding.setter)
        {
            binding.setter(tag, msg.value_double());
        }
    }

    void binding_set(
        const char* tag,
        TaggedBinding<std::int64_t>& binding,
        const proto::Binding& msg
    )
    {
        if (binding.setter)
        {
            binding.setter(tag, msg.value_number());
        }
    }

    void binding_set(
        const char* tag,
        TaggedBinding<std::string>& binding,
        const proto::Binding& msg
    )
    {
        if (binding.setter)
        {
            binding.setter(tag, msg.value_string());
        }
    }

    void binding_set(
        const char* tag,
        TaggedBinding<std::vector<std::uint8_t>>& binding,
        const proto::Binding& msg
    )
    {
        if (binding.setter)
        {
            const auto& buffer = msg.value_buffer();
            binding.setter(
                tag,
                // NOLINTNEXTLINE
                {reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()}
            );
        }
    }

    void msg_set(proto::Listener& msg, const Listener<void>& listener)
    {
        (void)msg;
        (void)listener;
    }

    void msg_set(proto::Listener& msg, const Listener<bool>& listener)
    {
        (void)listener;
        msg.set_type("arg_boolean");
    }

    void msg_set(proto::Listener& msg, const Listener<double>& listener)
    {
        (void)listener;
        msg.set_type("arg_double");
    }

    void msg_set(proto::Listener& msg, const Listener<std::int64_t>& listener)
    {
        (void)listener;
        msg.set_type("arg_number");
    }

    void msg_set(proto::Listener& msg, const Listener<std::string_view>& listener)
    {
        (void)listener;
        msg.set_type("arg_string");
    }

    void msg_set(proto::Listener& msg, const Listener<std::span<const std::uint8_t>>& listener)
    {
        (void)listener;
        msg.set_type("arg_buffer");
    }

    void msg_set(proto::Listener& msg, const TaggedListener<void>& listener)
    {
        (void)msg;
        (void)listener;
    }

    void msg_set(proto::Listener& msg, const TaggedListener<bool>& listener)
    {
        (void)listener;
        msg.set_type("arg_boolean");
    }

    void msg_set(proto::Listener& msg, const TaggedListener<double>& listener)
    {
        (void)listener;
        msg.set_type("arg_double");
    }

    void msg_set(proto::Listener& msg, const TaggedListener<std::int64_t>& listener)
    {
        (void)listener;
        msg.set_type("arg_number");
    }

    void msg_set(proto::Listener& msg, const TaggedListener<std::string_view>& listener)
    {
        (void)listener;
        msg.set_type("arg_string");
    }

    void msg_set(
        proto::Listener& msg,
        const TaggedListener<std::span<const std::uint8_t>>& listener
    )
    {
        (void)listener;
        msg.set_type("arg_buffer");
    }

    void invoke(const char* tag, const Listener<void>& listener, const proto::ListenerNotify& msg)
    {
        (void)tag;
        (void)msg;
        listener.on_change();
    }

    void invoke(const char* tag, const Listener<bool>& listener, const proto::ListenerNotify& msg)
    {
        (void)tag;
        listener.on_change(msg.arg_boolean());
    }

    void invoke(const char* tag, const Listener<double>& listener, const proto::ListenerNotify& msg)
    {
        (void)tag;
        listener.on_change(msg.arg_double());
    }

    void invoke(
        const char* tag,
        const Listener<std::int64_t>& listener,
        const proto::ListenerNotify& msg
    )
    {
        (void)tag;
        listener.on_change(msg.arg_number());
    }

    void invoke(
        const char* tag,
        const Listener<std::string_view>& listener,
        const proto::ListenerNotify& msg
    )
    {
        (void)tag;
        listener.on_change(msg.arg_string());
    }

    void invoke(
        const char* tag,
        const Listener<std::span<const std::uint8_t>>& listener,
        const proto::ListenerNotify& msg
    )
    {
        (void)tag;
        const auto& buffer = msg.arg_buffer();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        listener.on_change({reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()});
    }

    void invoke(
        const char* tag,
        const TaggedListener<void>& listener,
        const proto::ListenerNotify& msg
    )
    {
        (void)msg;
        listener.on_change(tag);
    }

    void invoke(
        const char* tag,
        const TaggedListener<bool>& listener,
        const proto::ListenerNotify& msg
    )
    {
        listener.on_change(tag, msg.arg_boolean());
    }

    void invoke(
        const char* tag,
        const TaggedListener<double>& listener,
        const proto::ListenerNotify& msg
    )
    {
        listener.on_change(tag, msg.arg_double());
    }

    void invoke(
        const char* tag,
        const TaggedListener<std::int64_t>& listener,
        const proto::ListenerNotify& msg
    )
    {
        listener.on_change(tag, msg.arg_number());
    }

    void invoke(
        const char* tag,
        const TaggedListener<std::string_view>& listener,
        const proto::ListenerNotify& msg
    )
    {
        listener.on_change(tag, msg.arg_string());
    }

    void invoke(
        const char* tag,
        const TaggedListener<std::span<const std::uint8_t>>& listener,
        const proto::ListenerNotify& msg
    )
    {
        const auto& buffer = msg.arg_buffer();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        listener.on_change(
            tag,
            // NOLINTNEXTLINE
            {reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()}
        );
    }
}
