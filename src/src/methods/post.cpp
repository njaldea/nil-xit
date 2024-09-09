#include <nil/xit/methods/post.hpp>

#include "../codec.hpp"
#include "../proto/message.pb.h"
#include "../structs.hpp"
#include "impl_set.hpp"

#include <nil/service/concat.hpp>

namespace nil::xit
{
    namespace impl
    {
        template <typename T>
        void post(Binding<T>& binding, T v)
        {
            nil::xit::proto::BindingUpdate msg;
            msg.set_id(binding.frame->id);

            auto* msg_binding = msg.mutable_binding();
            msg_binding->set_tag(binding.tag);
            msg_set(*msg_binding, std::move(v));

            const auto header = nil::xit::proto::MessageType_BindingUpdate;
            auto payload = nil::service::concat(header, msg);
            binding.frame->core->service->publish(std::move(payload));
        }
    }

    void post(Binding<std::int64_t>& b, std::int64_t v)
    {
        impl::post(b, v);
    }

    void post(Binding<std::string>& b, std::string v)
    {
        impl::post(b, std::move(v));
    }
}
