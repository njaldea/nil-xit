#include <nil/xit/tagged/post.hpp>

#include "structs.hpp"

#include "../codec.hpp"
#include "../proto/message.pb.h"
#include "../structs.hpp"
#include "../utils.hpp"

#include <nil/service.hpp>

namespace nil::xit::tagged
{
    namespace impl
    {
        template <typename T>
        void post(std::string_view tag, const Value<T>& value, T new_value)
        {
            proto::ValueUpdate msg;
            msg.set_id(value.frame->id);
            msg.set_tag(tag);

            auto* msg_value = msg.mutable_value();
            msg_value->set_id(value.id);
            nil::xit::utils::msg_set(std::move(new_value), *msg_value);

            const auto header = proto::MessageType_ValueUpdate;
            auto payload = nil::service::concat(header, msg);
            publish(*value.frame->core->service, std::move(payload));
        }
    }

    void post(std::string_view tag, const Value<bool>& value, bool new_value)
    {
        impl::post(tag, value, new_value);
    }

    void post(std::string_view tag, const Value<double>& value, double new_value)
    {
        impl::post(tag, value, new_value);
    }

    void post(std::string_view tag, const Value<std::int64_t>& value, std::int64_t new_value)
    {
        impl::post(tag, value, new_value);
    }

    void post(std::string_view tag, const Value<std::string>& value, std::string new_value)
    {
        impl::post(tag, value, std::move(new_value));
    }

    void post(
        std::string_view tag,
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value
    )
    {
        impl::post(tag, value, std::move(new_value));
    }
}
