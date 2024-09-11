#include "impl_set.hpp"
#include <type_traits>

namespace nil::xit::impl
{
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

    bool binding_set(std::int64_t& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(value, msg.value_i64());
    }

    bool binding_set(std::string& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(value, msg.value_str());
    }

    bool binding_set(std::vector<std::uint8_t>& value, const nil::xit::proto::Binding& msg)
    {
        return impl::binding_set(
            value,
            std::vector<std::uint8_t>(msg.value_buffer().begin(), msg.value_buffer().end())
        );
    }

    void msg_set(nil::xit::proto::Binding& msg, std::int64_t value)
    {
        msg.set_value_i64(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, const std::string& value)
    {
        msg.set_value_str(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, const std::vector<std::uint8_t>& value)
    {
        msg.set_value_buffer(value.data(), value.size());
    }
}
