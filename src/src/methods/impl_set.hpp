#pragma once

#include "../proto/message.pb.h"

namespace nil::xit::impl
{
    void msg_set(nil::xit::proto::Binding& msg, std::int64_t value);
    void msg_set(nil::xit::proto::Binding& msg, const std::string& value);
    void msg_set(nil::xit::proto::Binding& msg, const std::vector<std::uint8_t>& value);

    bool binding_set(std::int64_t& value, const nil::xit::proto::Binding& msg);
    bool binding_set(std::string& value, const nil::xit::proto::Binding& msg);
    bool binding_set(std::vector<std::uint8_t>& value, const nil::xit::proto::Binding& msg);
}
