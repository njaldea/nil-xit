#pragma once

#include "proto/message.pb.h"

namespace nil::xit::impl
{
    void msg_set(bool value, proto::Binding& msg, const char* tag);
    void msg_set(double value, proto::Binding& msg, const char* tag);
    void msg_set(std::int64_t value, proto::Binding& msg, const char* tag);
    void msg_set(std::string value, proto::Binding& msg, const char* tag);
    void msg_set(std::vector<std::uint8_t> value, proto::Binding& msg, const char* tag);
}
