#include "../codec.hpp"
#include "../proto/message.pb.h"
#include "../structs.hpp"
#include "impl_set.hpp"

#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <nil/service/map.hpp>

#include <fstream>

namespace nil::xit
{
    namespace impl
    {
        void handle(
            Core& core,
            const nil::service::ID& id,
            const nil::xit::proto::MarkupRequest& request
        )
        {
            const auto it = core.frames.find(request.id());
            if (it != core.frames.end())
            {
                nil::xit::proto::MarkupResponse response;
                response.set_id(it->first);
                response.set_file(it->second.path);

                const auto header = nil::xit::proto::MessageType_MarkupResponse;
                auto payload = nil::service::concat(header, response);
                core.service->send(id, std::move(payload));
            }
            else
            {
                // error response
            }
        }

        void handle(
            Core& core,
            const nil::service::ID& id,
            const nil::xit::proto::BindingRequest& request
        )
        {
            const auto it = core.frames.find(request.id());
            if (it != core.frames.end())
            {
                nil::xit::proto::BindingResponse response;
                response.set_id(it->first);
                const auto& frame = it->second;
                for (const auto& [tag, binding] : frame.bindings)
                {
                    auto* msg_binding = response.add_bindings();
                    msg_binding->set_tag(tag);
                    std::visit(
                        [msg_binding](const auto& v) { impl::msg_set(*msg_binding, v.value); },
                        binding
                    );
                }

                const auto header = nil::xit::proto::MessageType_BindingResponse;
                auto payload = nil::service::concat(header, response);
                core.service->send(id, std::move(payload));
            }
            else
            {
                // error response
            }
        }

        void handle(
            Core& core,
            const nil::service::ID& /* id */,
            const nil::xit::proto::BindingUpdate& msg
        )
        {
            auto it = core.frames.find(msg.id());
            if (it != core.frames.end())
            {
                auto binding_it = it->second.bindings.find(msg.binding().tag());
                if (binding_it != it->second.bindings.end())
                {
                    std::visit(
                        [&msg](auto& b)
                        {
                            if (binding_set(b.value, msg.binding()) && b.on_change)
                            {
                                b.on_change(b.value);
                            }
                        },
                        binding_it->second
                    );
                }
            }
            else
            {
                // error response
            }
        }

        void handle(
            Core& core,
            const nil::service::ID& id,
            const nil::xit::proto::ListenerRequest& request
        )
        {
            const auto it = core.frames.find(request.id());
            if (it != core.frames.end())
            {
                nil::xit::proto::ListenerResponse response;
                response.set_id(it->first);
                const auto& frame = it->second;
                for (const auto& [tag, listener] : frame.listeners)
                {
                    auto* msg_listener = response.add_listeners();
                    msg_listener->set_tag(tag);
                    std::visit([=](const auto& l) { msg_set(*msg_listener, l); }, listener);
                }

                const auto header = nil::xit::proto::MessageType_ListenerResponse;
                auto payload = nil::service::concat(header, response);
                core.service->send(id, std::move(payload));
            }
            else
            {
                // error response
            }
        }

        void handle(
            Core& core,
            const nil::service::ID& /* id */,
            const nil::xit::proto::ListenerNotify& msg
        )
        {
            const auto it = core.frames.find(msg.id());
            if (it != core.frames.end())
            {
                auto& listeners = it->second.listeners;
                auto lit = listeners.find(msg.tag());
                if (lit != listeners.end())
                {
                    std::visit([&](const auto& listener) { invoke(listener, msg); }, lit->second);
                }
            }
            else
            {
                // error response
            }
        }

        void handle(
            Core& core,
            const nil::service::ID& id,
            const nil::xit::proto::FileRequest& request
        )
        {
            nil::xit::proto::FileResponse response;
            response.set_target(request.target());

            std::fstream file(request.target());
            response.set_content(std::string( //
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>()
            ));

            const auto header = nil::xit::proto::MessageType_FileResponse;
            auto payload = nil::service::concat(header, response);
            core.service->send(id, std::move(payload));
        }

        void install_on_message(Core& core)
        {
            auto make_handler = [ptr = &core](auto consume)
            {
                return [ptr, consume](const auto& id, const void* data, std::uint64_t size)
                { handle(*ptr, id, consume(data, size)); };
            };
            auto handlers            //
                = nil::service::map( //
                    nil::service::mapping(
                        nil::xit::proto::MessageType_MarkupRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::MarkupRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_BindingRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::BindingRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_ListenerRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::ListenerRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_FileRequest,
                        make_handler(&nil::service::consume<nil::xit::proto::FileRequest>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_BindingUpdate,
                        make_handler(&nil::service::consume<nil::xit::proto::BindingUpdate>)
                    ),
                    nil::service::mapping(
                        nil::xit::proto::MessageType_ListenerNotify,
                        make_handler(&nil::service::consume<nil::xit::proto::ListenerNotify>)
                    )
                );
            core.service->on_message(std::move(handlers));
        }
    }

    C make_core(nil::service::IService& service)
    {
        constexpr auto deleter = [](Core* obj) { std::default_delete<Core>()(obj); };
        auto holder = std::make_unique<Core>(&service);
        impl::install_on_message(*holder);
        return {{holder.release(), deleter}};
    }
}
