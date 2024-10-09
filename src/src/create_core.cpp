#include "codec.hpp"
#include "proto/message.pb.h"
#include "structs.hpp"

#include "tagged/utils.hpp"
#include "unique/utils.hpp"

#include <nil/service.hpp>

#include <filesystem>
#include <fstream>

namespace
{
    std::string metadata(auto i)
    {
        using i_t = decltype(i);

        const union
        {
            i_t i;
            char c[sizeof(i_t)]; // NOLINT
        } m = {.i = i};

        return std::string(&m.c[0], sizeof(i_t)); // NOLINT
    }

    struct MsgSetBindingMapper
    {
        template <typename T>
        void operator()(const nil::xit::unique::Binding<T>& binding) const
        {
            nil::xit::unique::impl::msg_set(binding, *msg, tag);
        }

        template <typename T>
        void operator()(const nil::xit::tagged::Binding<T>& binding) const
        {
            nil::xit::tagged::impl::msg_set(binding, *msg, tag);
        }

        nil::xit::proto::Binding* msg;
        const char* tag;
    };

    struct MsgSetListenerMapper
    {
        template <typename T>
        void operator()(const nil::xit::unique::Listener<T>& listener) const
        {
            nil::xit::unique::impl::msg_set(listener, *msg);
        }

        template <typename T>
        void operator()(const nil::xit::tagged::Listener<T>& listener) const
        {
            nil::xit::tagged::impl::msg_set(listener, *msg);
        }

        nil::xit::proto::Listener* msg;
    };

    struct InvokeListenerMapper
    {
        template <typename T>
        void operator()(const nil::xit::unique::Listener<T>& listener) const
        {
            nil::xit::unique::impl::invoke(listener, *msg, tag);
        }

        template <typename T>
        void operator()(const nil::xit::tagged::Listener<T>& listener) const
        {
            nil::xit::tagged::impl::invoke(listener, *msg, tag);
        }

        const nil::xit::proto::ListenerNotify* msg;
        const char* tag;
    };

    struct BindingSetMapper
    {
        template <typename T>
        void operator()(nil::xit::unique::Binding<T>& binding) const
        {
            nil::xit::unique::impl::binding_set(binding, *msg, tag);
        }

        template <typename T>
        void operator()(nil::xit::tagged::Binding<T>& binding) const
        {
            nil::xit::tagged::impl::binding_set(binding, *msg, tag);
        }

        const nil::xit::proto::Binding* msg;
        const char* tag;
    };
}

namespace nil::xit::impl
{
    void handle(Core& core, const nil::service::ID& id, const proto::FrameRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            const auto cached_file = core.cache_location / request.id();
            if (std::filesystem::exists(cached_file))
            {
                proto::FrameCache cache;
                {
                    std::ifstream f(cached_file, std::ios::binary | std::ios::in);
                    cache.ParseFromIstream(&f);
                }
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
                    send(core.service, id, std::move(payload));
                    return;
                }
            }

            proto::FrameResponse response;
            response.set_id(it->first);
            std::visit(
                [&response](const auto& f) { response.set_file(f.path.string()); },
                it->second
            );

            const auto header = proto::MessageType_FrameResponse;
            auto payload = nil::service::concat(header, response);
            send(core.service, id, std::move(payload));
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& id, const proto::BindingRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            proto::BindingResponse response;
            response.set_id(it->first);
            const char* tag = request.has_tag() ? request.tag().data() : nullptr;
            if (tag != nullptr)
            {
                response.set_tag(tag);
            }
            std::visit(
                [&response, tag](const auto& frame)
                {
                    for (const auto& [binding_id, binding] : frame.bindings)
                    {
                        auto* msg_binding = response.add_bindings();
                        msg_binding->set_id(binding_id);
                        std::visit(MsgSetBindingMapper(msg_binding, tag), binding);
                    }
                },
                it->second
            );

            const auto header = proto::MessageType_BindingResponse;
            auto payload = nil::service::concat(header, response);
            send(core.service, id, std::move(payload));
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::FrameCache& msg)
    {
        const auto it = core.frames.find(msg.id());
        if (it != core.frames.end())
        {
            std::ofstream f(core.cache_location / msg.id(), std::ios::binary | std::ios::out);
            msg.SerializeToOstream(&f);
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::BindingUpdate& msg)
    {
        auto it = core.frames.find(msg.id());
        if (it != core.frames.end())
        {
            std::visit(
                [&msg](auto& frame)
                {
                    const char* tag = msg.has_tag() ? msg.tag().data() : nullptr;
                    auto binding_it = frame.bindings.find(msg.binding().id());
                    if (binding_it != frame.bindings.end())
                    {
                        std::visit(BindingSetMapper(&msg.binding(), tag), binding_it->second);
                    }
                },
                it->second
            );
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& id, const proto::ListenerRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            proto::ListenerResponse response;
            response.set_id(it->first);
            std::visit(
                [&response](const auto& frame)
                {
                    for (const auto& [listener_id, listener] : frame.listeners)
                    {
                        auto* msg_listener = response.add_listeners();
                        msg_listener->set_id(listener_id);
                        std::visit(MsgSetListenerMapper(msg_listener), listener);
                    }
                },
                it->second
            );

            const auto header = proto::MessageType_ListenerResponse;
            auto payload = nil::service::concat(header, response);
            send(core.service, id, std::move(payload));
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::ListenerNotify& msg)
    {
        const auto it = core.frames.find(msg.frame_id());
        if (it != core.frames.end())
        {
            std::visit(
                [&msg](const auto& frame)
                {
                    const char* tag = msg.has_tag() ? msg.tag().data() : nullptr;
                    auto& listeners = frame.listeners;
                    auto lit = listeners.find(msg.listener_id());
                    if (lit != listeners.end())
                    {
                        std::visit(InvokeListenerMapper(&msg, tag), lit->second);
                    }
                },
                it->second
            );
            return;
        }
    }

    void handle(Core& core, const nil::service::ID& id, const proto::FileRequest& request)
    {
        proto::FileResponse response;
        response.set_target(request.target());

        std::ifstream file(request.target(), std::ios::binary | std::ios::in);
        response.set_content(std::string( //
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        ));

        response.set_metadata(::metadata(
            std::filesystem::last_write_time(request.target()).time_since_epoch().count()
        ));
        auto header = proto::MessageType_FileResponse;
        auto payload = nil::service::concat(header, response);
        send(core.service, id, std::move(payload));
    }

    auto make_handlers(Core& core)
    {
        auto make_handler = [ptr = &core](auto consume)
        {
            return [ptr, consume](const auto& id, const void* data, std::uint64_t size)
            { handle(*ptr, id, consume(data, size)); };
        };
        return nil::service::map( //
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
    }
}

namespace nil::xit
{
    C create_core(nil::service::S service)
    {
        constexpr auto deleter = [](Core* obj) { std::default_delete<Core>()(obj); };
        auto holder = std::make_unique<Core>(service, std::filesystem::temp_directory_path());
        on_message(service, impl::make_handlers(*holder));
        on_ready(
            service,
            [ptr = holder.get()]() { std::filesystem::create_directories(ptr->cache_location); }
        );
        return {{holder.release(), deleter}};
    }

    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path)
    {
        core.cache_location = tmp_path / "nil/xit";
    }
}
