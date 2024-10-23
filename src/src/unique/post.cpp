#include <nil/xit/unique/post.hpp>

#include "structs.hpp"

#include "../codec.hpp"
#include "../proto/message.pb.h"
#include "../structs.hpp"
#include "../utils.hpp"

#include <nil/service.hpp>

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T>
        void post(const Value<T>& value, T v)
        {
            proto::ValueUpdate msg;
            msg.set_id(value.frame->id);

            auto* msg_value = msg.mutable_value();
            msg_value->set_id(value.id);
            nil::xit::utils::msg_set(std::move(v), *msg_value);

            const auto header = proto::MessageType_ValueUpdate;
            auto payload = nil::service::concat(header, msg);
            publish(*value.frame->core->service, std::move(payload));
        }
    }

    void post(const Value<bool>& value, bool new_value)
    {
        impl::post(value, new_value);
    }

    void post(const Value<double>& value, double new_value)
    {
        impl::post(value, new_value);
    }

    void post(const Value<std::int64_t>& value, std::int64_t new_value)
    {
        impl::post(value, new_value);
    }

    void post(const Value<std::string>& value, std::string new_value)
    {
        impl::post(value, std::move(new_value));
    }

    void post(const Value<std::vector<std::uint8_t>>& value, std::vector<std::uint8_t> new_value)
    {
        impl::post(value, std::move(new_value));
    }
}
