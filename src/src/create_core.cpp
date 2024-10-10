#include "codec.hpp"
#include "proto/message.pb.h"
#include "structs.hpp"

#include "tagged/utils.hpp"
#include "unique/structs.hpp"
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

    struct MsgSetValueMapper
    {
        template <typename T>
        void operator()(const nil::xit::unique::Value<T>& value) const
        {
            nil::xit::unique::impl::msg_set(value, *msg, tag);
        }

        template <typename T>
        void operator()(const nil::xit::tagged::Value<T>& value) const
        {
            nil::xit::tagged::impl::msg_set(value, *msg, tag);
        }

        nil::xit::proto::Value* msg;
        const char* tag;
    };

    struct MsgSetSignalMapper
    {
        template <typename T>
        void operator()(const nil::xit::unique::Signal<T>& signal) const
        {
            nil::xit::unique::impl::msg_set(signal, *msg);
        }

        template <typename T>
        void operator()(const nil::xit::tagged::Signal<T>& signal) const
        {
            nil::xit::tagged::impl::msg_set(signal, *msg);
        }

        nil::xit::proto::Signal* msg;
    };

    struct InvokeSignalMapper
    {
        template <typename T>
        void operator()(const nil::xit::unique::Signal<T>& signal) const
        {
            nil::xit::unique::impl::invoke(signal, *msg, tag);
        }

        template <typename T>
        void operator()(const nil::xit::tagged::Signal<T>& signal) const
        {
            nil::xit::tagged::impl::invoke(signal, *msg, tag);
        }

        const nil::xit::proto::SignalNotify* msg;
        const char* tag;
    };

    struct ValueSetMapper
    {
        template <typename T>
        void operator()(nil::xit::unique::Value<T>& value) const
        {
            nil::xit::unique::impl::value_set(value, *msg, tag);
        }

        template <typename T>
        void operator()(nil::xit::tagged::Value<T>& value) const
        {
            nil::xit::tagged::impl::value_set(value, *msg, tag);
        }

        const nil::xit::proto::Value* msg;
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
                    send(*core.service, id, std::move(payload));
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
            send(*core.service, id, std::move(payload));
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& id, const proto::ValueRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            proto::ValueResponse response;
            response.set_id(it->first);
            const char* tag = request.has_tag() ? request.tag().data() : nullptr;
            if (tag != nullptr)
            {
                response.set_tag(tag);
            }
            std::visit(
                [&response, tag](const auto& frame)
                {
                    for (const auto& [value_id, value] : frame.values)
                    {
                        auto* msg_value = response.add_values();
                        msg_value->set_id(value_id);
                        std::visit(MsgSetValueMapper(msg_value, tag), value);
                    }
                },
                it->second
            );

            const auto header = proto::MessageType_ValueResponse;
            auto payload = nil::service::concat(header, response);
            send(*core.service, id, std::move(payload));
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

    void handle(Core& core, const nil::service::ID& /* id */, const proto::ValueUpdate& msg)
    {
        auto it = core.frames.find(msg.id());
        if (it != core.frames.end())
        {
            std::visit(
                [&msg](auto& frame)
                {
                    const char* tag = msg.has_tag() ? msg.tag().data() : nullptr;
                    auto value_it = frame.values.find(msg.value().id());
                    if (value_it != frame.values.end())
                    {
                        std::visit(ValueSetMapper(&msg.value(), tag), value_it->second);
                    }
                },
                it->second
            );
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& id, const proto::SignalRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            proto::SignalResponse response;
            response.set_id(it->first);
            std::visit(
                [&response](const auto& frame)
                {
                    for (const auto& [signal_id, signal] : frame.signals)
                    {
                        auto* msg_signal = response.add_signals();
                        msg_signal->set_id(signal_id);
                        std::visit(MsgSetSignalMapper(msg_signal), signal);
                    }
                },
                it->second
            );

            const auto header = proto::MessageType_SignalResponse;
            auto payload = nil::service::concat(header, response);
            send(*core.service, id, std::move(payload));
            return;
        }
        // error response
    }

    void handle(Core& core, const nil::service::ID& /* id */, const proto::SignalNotify& msg)
    {
        const auto it = core.frames.find(msg.frame_id());
        if (it != core.frames.end())
        {
            std::visit(
                [&msg](const auto& frame)
                {
                    const char* tag = msg.has_tag() ? msg.tag().data() : nullptr;
                    auto lit = frame.signals.find(msg.signal_id());
                    if (lit != frame.signals.end())
                    {
                        std::visit(InvokeSignalMapper(&msg, tag), lit->second);
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
        send(*core.service, id, std::move(payload));
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
                proto::MessageType_ValueRequest,
                make_handler(&nil::service::consume<proto::ValueRequest>)
            ),
            nil::service::mapping(
                proto::MessageType_SignalRequest,
                make_handler(&nil::service::consume<proto::SignalRequest>)
            ),
            nil::service::mapping(
                proto::MessageType_FileRequest,
                make_handler(&nil::service::consume<proto::FileRequest>)
            ),
            nil::service::mapping(
                proto::MessageType_ValueUpdate,
                make_handler(&nil::service::consume<proto::ValueUpdate>)
            ),
            nil::service::mapping(
                proto::MessageType_SignalNotify,
                make_handler(&nil::service::consume<proto::SignalNotify>)
            )
        );
    }
}

namespace nil::xit
{
    C create_core(nil::service::S service)
    {
        using namespace std::filesystem;
        constexpr auto deleter = [](Core* obj) { std::default_delete<Core>()(obj); };
        auto holder = std::make_unique<Core>(
            &static_cast<nil::service::MessagingService&>(service),
            temp_directory_path() / "nil/xit"
        );
        on_message(service, impl::make_handlers(*holder));
        on_ready(service, [ptr = holder.get()]() { create_directories(ptr->cache_location); });
        return {{holder.release(), deleter}};
    }

    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path)
    {
        core.cache_location = tmp_path / "nil/xit";
    }
}
