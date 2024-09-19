#include <nil/xit/methods/post.hpp>

#include "../codec.hpp"
#include "../proto/message.pb.h"
#include "../structs.hpp"
#include "impl_set.hpp"

#include <nil/service.hpp>

namespace nil::xit
{
    namespace impl
    {
        template <typename T>
        void post(const Binding<T>& binding, T v)
        {
            proto::BindingUpdate msg;
            msg.set_id(binding.frame->id);

            auto* msg_binding = msg.mutable_binding();
            msg_binding->set_tag(binding.tag);
            msg_set(*msg_binding, std::move(v));

            const auto header = proto::MessageType_BindingUpdate;
            auto payload = nil::service::concat(header, msg);
            publish(binding.frame->core->service, std::move(payload));
        }
    }

    void post(const Binding<bool>& binding, bool value)
    {
        impl::post(binding, value);
    }

    void post(const Binding<double>& binding, double value)
    {
        impl::post(binding, value);
    }

    void post(const Binding<std::int64_t>& binding, std::int64_t value)
    {
        impl::post(binding, value);
    }

    void post(const Binding<std::string>& binding, std::string value)
    {
        impl::post(binding, std::move(value));
    }

    void post(const Binding<std::vector<std::uint8_t>>& binding, std::vector<std::uint8_t> value)
    {
        impl::post(binding, std::move(value));
    }
}
