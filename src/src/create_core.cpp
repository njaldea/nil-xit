#include "codec.hpp"
#include "proto/message.pb.h"
#include "structs.hpp"

#include "tagged/utils.hpp"   // IWYU pragma: keep
#include "unique/structs.hpp" // IWYU pragma: keep
#include "unique/utils.hpp"   // IWYU pragma: keep

#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <nil/service/map.hpp>
#include <nil/service/structs.hpp>

#include <filesystem>
#include <fstream>
#include <optional>

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
}

namespace nil::xit::impl
{
    void handle(Core& core, const nil::service::ID& id, const proto::FrameRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            auto& [frame_id, frame] = *it;
            const auto cached_file = core.cache_location / frame_id;
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
                    response.set_id(frame_id);
                    response.set_content(cache.content());

                    const auto header = proto::MessageType_FrameResponse;
                    auto payload = nil::service::concat(header, response);
                    send(*core.service, id, std::move(payload));
                }
            }

            proto::FrameResponse response;
            response.set_id(frame_id);
            std::visit(
                [&core, &response](const auto& f)
                {
                    if (core.directory.has_value())
                    {
                        response.set_file((*core.directory / f.path).string());
                    }
                    else
                    {
                        response.set_file(f.path.string());
                    }
                },
                frame
            );

            const auto header = proto::MessageType_FrameResponse;
            auto payload = nil::service::concat(header, response);
            send(*core.service, id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const proto::ValueRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            auto& [frame_id, frame] = *it;
            proto::ValueResponse response;
            response.set_id(frame_id);
            const char* tag = request.has_tag() ? request.tag().data() : nullptr;
            if (tag != nullptr)
            {
                response.set_tag(tag);
            }
            std::visit(
                [&response, tag](const auto& f)
                {
                    for (const auto& [value_id, value] : f.values)
                    {
                        auto* msg = response.add_values();
                        msg->set_id(value_id);
                        std::visit([tag, msg](const auto& v) { msg_set(v, *msg, tag); }, value);
                    }
                },
                frame
            );

            const auto header = proto::MessageType_ValueResponse;
            auto payload = nil::service::concat(header, response);
            send(*core.service, id, std::move(payload));
        }
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
                    auto v_it = frame.values.find(msg.value().id());
                    if (v_it != frame.values.end())
                    {
                        auto& v = v_it->second;
                        std::visit([&msg, tag](auto& vv) { value_set(vv, msg.value(), tag); }, v);
                    }
                },
                it->second
            );
        }
    }

    void handle(Core& core, const nil::service::ID& id, const proto::SignalRequest& request)
    {
        const auto it = core.frames.find(request.id());
        if (it != core.frames.end())
        {
            auto& [frame_id, frame] = *it;
            proto::SignalResponse response;
            response.set_id(frame_id);
            std::visit(
                [&response](const auto& f)
                {
                    for (const auto& [signal_id, signal] : f.signals)
                    {
                        auto* msg = response.add_signals();
                        msg->set_id(signal_id);
                        std::visit([msg](const auto& s) { msg_set(s, *msg); }, signal);
                    }
                },
                frame
            );

            const auto header = proto::MessageType_SignalResponse;
            auto payload = nil::service::concat(header, response);
            send(*core.service, id, std::move(payload));
        }
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
                    auto s_it = frame.signals.find(msg.signal_id());
                    if (s_it != frame.signals.end())
                    {
                        auto& s = s_it->second;
                        std::visit([&msg, tag](const auto& ss) { invoke(ss, msg, tag); }, s);
                    }
                },
                it->second
            );
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

    struct Mapper
    {
        Core* core;

        template <typename T>
        auto map(proto::MessageType type) const
        {
            return nil::service::mapping(
                type,
                [core = this->core](const auto& id, const void* data, std::uint64_t size)
                { handle(*core, id, nil::service::consume<T>(data, size)); }
            );
        }
    };
}

namespace nil::xit
{
    Core* create_core(nil::service::S service)
    {
        using namespace std::filesystem;
        Core* ptr = new Core(
            &static_cast<nil::service::MessagingService&>(service),
            temp_directory_path() / "nil/xit",
            std::nullopt,
            std::unordered_map<std::string, std::variant<unique::Frame, tagged::Frame>>()
        );
        const auto mapper = impl::Mapper{ptr};
        on_message(
            service,
            nil::service::map(
                mapper.map<proto::FrameRequest>(proto::MessageType_FrameRequest),
                mapper.map<proto::FrameCache>(proto::MessageType_FrameCache),
                mapper.map<proto::ValueRequest>(proto::MessageType_ValueRequest),
                mapper.map<proto::SignalRequest>(proto::MessageType_SignalRequest),
                mapper.map<proto::FileRequest>(proto::MessageType_FileRequest),
                mapper.map<proto::ValueUpdate>(proto::MessageType_ValueUpdate),
                mapper.map<proto::SignalNotify>(proto::MessageType_SignalNotify)
            )
        );
        on_ready(service, [ptr]() { create_directories(ptr->cache_location); });
        return ptr;
    }

    void delete_core(Core* core)
    {
        std::default_delete<Core>()(core);
    }

    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path)
    {
        core.cache_location = tmp_path / "nil/xit";
    }

    void set_relative_directory(Core& core, const std::filesystem::path& directory)
    {
        core.directory = directory;
    }
}
