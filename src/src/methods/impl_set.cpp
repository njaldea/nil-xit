#include "impl_set.hpp"

namespace nil::xit::impl
{
    namespace impl
    {
        template <typename T>
        bool binding_set(T& value_out, T value_in)
        {
            if (value_out != value_in)
            {
                value_out = std::move(value_in);
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

    void msg_set(nil::xit::proto::Binding& msg, std::int64_t value)
    {
        msg.set_value_i64(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, std::string value)
    {
        msg.set_value_str(std::move(value));
    }
}
