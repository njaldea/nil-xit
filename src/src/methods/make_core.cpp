#include "../codec.hpp"
#include "../proto/message.pb.h"
#include "../structs.hpp"
#include "impl_set.hpp"

#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <nil/service/map.hpp>

#include <filesystem>
#include <fstream>

namespace
{
    std::string metadata(std::int64_t i)
    {
        union Metadata
        {
            std::int64_t i;
            char c[sizeof(std::int64_t)]; // NOLINT
        } m = {.i = i};

        return std::string(&m.c[0], sizeof(std::int64_t)); // NOLINT
    }
}

namespace nil::xit::impl
{
    void handle(Core& core, const nil::service::ID& id, const proto::FrameRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            const auto tmp = std::filesystem::temp_directory_path() / "nil/xit";
            if (std::filesystem::exists(tmp / request.id()))
            {
                proto::FrameCache cache;

                std::ifstream f(tmp / request.id(), std::ios::binary | std::ios::in);
                cache.ParseFromIstream(&f);

                bool cached = true;
                for (const auto& ff : cache.files())
                {
                    if (!std::filesystem::exists(ff.target()))
                    {
                        cached = false;
                        break;
                    }

                    const auto metadata = ::metadata(
                        std::filesystem::last_write_time(ff.target()).time_since_epoch().count()
                    );
                    if (metadata != ff.metadata())
                    {
                        cached = false;
                        break;
                    }
                }
                if (cached)
                {
                    proto::FrameResponse response;
                    response.set_id(it->first);
                    response.set_content(cache.content());

                    const auto header = proto::MessageType_FrameResponse;
                    auto payload = nil::service::concat(header, response);
                    core.service->send(id, std::move(payload));
                    return;
                }
            }

            proto::FrameResponse response;
            response.set_id(it->first);
            response.set_file(it->second.path);

            const auto header = proto::MessageType_FrameResponse;
            auto payload = nil::service::concat(header, response);
            core.service->send(id, std::move(payload));
        }
        else
        {
            // error response
        }
    }

    void handle(Core& core, const nil::service::ID& id, const proto::BindingRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            proto::BindingResponse response;
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

            const auto header = proto::MessageType_BindingResponse;
            auto payload = nil::service::concat(header, response);
            core.service->send(id, std::move(payload));
        }
        else
        {
            // error response
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::FrameCache& msg)
    {
        const auto it = core.frames.find(msg.id());
        if (it != core.frames.end())
        {
            const auto tmp = std::filesystem::temp_directory_path() / "nil/xit";
            std::filesystem::create_directories(tmp);
            std::ofstream f(tmp / msg.id(), std::ios::binary | std::ios::out);
            msg.SerializeToOstream(&f);
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::BindingUpdate& msg)
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

    void handle(Core& core, const nil::service::ID& id, const proto::ListenerRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            proto::ListenerResponse response;
            response.set_id(it->first);
            const auto& frame = it->second;
            for (const auto& [tag, listener] : frame.listeners)
            {
                auto* msg_listener = response.add_listeners();
                msg_listener->set_tag(tag);
                std::visit([=](const auto& l) { msg_set(*msg_listener, l); }, listener);
            }

            const auto header = proto::MessageType_ListenerResponse;
            auto payload = nil::service::concat(header, response);
            core.service->send(id, std::move(payload));
        }
        else
        {
            // error response
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::ListenerNotify& msg)
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

    void handle(Core& core, const nil::service::ID& id, const proto::FileRequest& request)
    {
        proto::FileResponse response;
        response.set_target(request.target());

        std::fstream file(request.target());
        response.set_content(std::string( //
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        ));

        response.set_metadata(::metadata(
            std::filesystem::last_write_time(request.target()).time_since_epoch().count()
        ));
        auto header = proto::MessageType_FileResponse;
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
                    proto::MessageType_FrameRequest,
                    make_handler(&nil::service::consume<proto::FrameRequest>)
                ),
                nil::service::mapping(
                    proto::MessageType_FrameCache,
                    make_handler(&nil::service::consume<proto::FrameCache>)
                ),
                nil::service::mapping(
                    proto::MessageType_BindingRequest,
                    make_handler(&nil::service::consume<proto::BindingRequest>)
                ),
                nil::service::mapping(
                    proto::MessageType_ListenerRequest,
                    make_handler(&nil::service::consume<proto::ListenerRequest>)
                ),
                nil::service::mapping(
                    proto::MessageType_FileRequest,
                    make_handler(&nil::service::consume<proto::FileRequest>)
                ),
                nil::service::mapping(
                    proto::MessageType_BindingUpdate,
                    make_handler(&nil::service::consume<proto::BindingUpdate>)
                ),
                nil::service::mapping(
                    proto::MessageType_ListenerNotify,
                    make_handler(&nil::service::consume<proto::ListenerNotify>)
                )
            );
        core.service->on_message(std::move(handlers));
    }
}

namespace nil::xit
{
    C make_core(nil::service::IService& service)
    {
        constexpr auto deleter = [](Core* obj) { std::default_delete<Core>()(obj); };
        auto holder = std::make_unique<Core>(&service);
        impl::install_on_message(*holder);
        return {{holder.release(), deleter}};
    }
}
