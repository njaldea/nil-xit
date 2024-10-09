#include "utils.hpp"

namespace nil::xit::impl
{
    void msg_set(bool value, proto::Binding& msg, const char* tag)
    {
        (void)tag;
        msg.set_value_boolean(value);
    }

    void msg_set(double value, proto::Binding& msg, const char* tag)
    {
        (void)tag;
        msg.set_value_double(value);
    }

    void msg_set(std::int64_t value, proto::Binding& msg, const char* tag)
    {
        (void)tag;
        msg.set_value_number(value);
    }

    void msg_set(std::string value, proto::Binding& msg, const char* tag)
    {
        (void)tag;
        msg.set_value_string(std::move(value));
    }

    void msg_set(std::vector<std::uint8_t> value, proto::Binding& msg, const char* tag)
    {
        (void)tag;
        msg.set_value_buffer(value.data(), value.size());
    }
}
