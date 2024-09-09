#include "msg_set.hpp"

namespace nil::xit::impl
{
    void msg_set(nil::xit::proto::Binding& msg, std::int64_t value)
    {
        msg.set_value_i64(value);
    }

    void msg_set(nil::xit::proto::Binding& msg, std::string value)
    {
        msg.set_value_str(std::move(value));
    }
}
